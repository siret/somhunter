import os
import struct
import numpy as np
import mxnet as mx
from sklearn import decomposition
from collections import namedtuple


class W2VVFeatureExtractor:

    def __init__(self, networks, image_weight_matrix: str, image_bias_vector: str, n_components: int=128, name="w2vv"):
        self._forward_fcs = []
        for net in networks:
            self._forward_fcs.append(
                self._get_network_fc(net["network_path"], net["network_epoch"], net["normalize_inputs"])
            )

        self._image_weight_matrix = image_weight_matrix
        self._image_bias_vector = image_bias_vector
        self._n_components = n_components
        self.__name__ = name

    @staticmethod
    def _get_network_fc(network_path, network_epoch, normalize_inputs):
        batch_def = namedtuple('Batch', ['data'])
        sym, arg_params, aux_params = mx.model.load_checkpoint(network_path, network_epoch)

        network = mx.mod.Module(symbol=sym.get_internals()['flatten0_output'],
                                label_names=None,
                                context=mx.gpu())
        network.bind(for_training=False,
                     data_shapes=[("data", (1, 3, 224, 224))])
        network.set_params(arg_params, aux_params)

        def fc(image):
            image = image.astype(np.float32)
            if normalize_inputs:  # true for resnext101
                image = image - np.array([[[123.68, 116.779, 103.939]]], dtype=np.float32)
            image = np.transpose(image, [2, 0, 1])[np.newaxis]
            inputs = batch_def([mx.nd.array(image)])

            network.forward(inputs)
            return network.get_outputs()[0].asnumpy()

        return fc

    def predict_and_save(self, frames: np.ndarray, save_dir: str):
        assert frames.dtype == np.uint8 and len(frames.shape) == 4, \
            "Input shape must be [frames, height, width, 3] and type np.uint8."

        features = []
        for frame_idx in range(len(frames)):
            vector = np.concatenate([fc(frames[frame_idx]) for fc in self._forward_fcs], 1)
            features.append(vector)
        features = np.concatenate(features, 0)

        np.save(os.path.join(save_dir, self.__name__ + ".npy"), features)
        return features

    def load_result_from_disk(self, save_dir: str):
        return np.load(os.path.join(save_dir, self.__name__ + ".npy"))

    def merge(self, videos, no_frames_per_video, save_file):
        no_frames_total = sum(no_frames_per_video)

        idx = 0
        data = None
        for i, (video_dir, no_frames) in enumerate(zip(videos, no_frames_per_video)):
            features = self.load_result_from_disk(video_dir)

            if i == 0:
                dim = features.shape[1]
                data = np.empty([no_frames_total, dim], np.float32)

            data[idx:idx + no_frames] = features
            idx += no_frames

        img_weight = np.load(self._image_weight_matrix)
        img_bias = np.load(self._image_bias_vector)

        vis_vecs = np.tanh(np.matmul(img_weight, data.T).T + img_bias.reshape([1, -1]))
        norm = np.linalg.norm(vis_vecs, axis=1, keepdims=True)
        vis_vecs_normed = vis_vecs / norm

        pca = decomposition.PCA(n_components=self._n_components)
        vis_vecs_normed_pca = pca.fit_transform(vis_vecs_normed)

        norm = np.linalg.norm(vis_vecs_normed_pca, axis=1, keepdims=True)
        vis_vecs_normed_pca = vis_vecs_normed_pca / norm

        with open(save_file + f".w2vv.normed.{self._n_components}pca.viretformat", "wb") as f:
            f.write(struct.pack("<I", vis_vecs_normed_pca.shape[0]))
            f.write(struct.pack("<I", vis_vecs_normed_pca.shape[1] * 4))

            # metadata size
            f.write(struct.pack("<I", 0))

            for features in vis_vecs_normed_pca:
                f.write(struct.pack("<" + "f" * len(features), *features))

        # np.save(save_file + "." + self.__name__ + ".pca.matrix.npy", pca.components_)
        # np.save(save_file + "." + self.__name__ + ".pca.mean.npy", pca.mean_)

        with open(save_file + "." + self.__name__ + ".pca.matrix.bin", "wb") as f:
            f.write(pca.components_.tobytes("C"))
        with open(save_file + "." + self.__name__ + ".pca.mean.bin", "wb") as f:
            f.write(pca.mean_.tobytes("C"))
