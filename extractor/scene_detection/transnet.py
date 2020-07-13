import os
import logging
import numpy as np

from utils import tf_utils
from .transnet_utils import get_scenes_from_predictions, visualize_predictions

logger = logging.getLogger(__file__)


class TransNetV1:

    def __init__(self, proto_file, threshold: float=0.1, name="transnet"):
        self.threshold = threshold
        self._session = tf_utils.load_graph_from_proto(proto_file)

        self._inputs = self._session.graph.get_tensor_by_name("import/TransNet/Inputs:0")
        self._predictions = self._session.graph.get_tensor_by_name("import/TransNet/Predictions:0")
        self.__name__ = name

    def _predict_raw(self, frames: np.ndarray):
        assert len(frames.shape) == 5, "Input shape must be [batch, frames, height, width, 3]."
        return self._session.run(self._predictions, feed_dict={self._inputs: frames})

    def predict_and_save(self, frames: np.ndarray, save_dir: str):
        assert len(frames.shape) == 4, "Input shape must be [frames, height, width, 3]."

        def input_iterator():
            # return windows of size 100 where the first/last 25 frames are from the previous/next batch
            # the first and last window must be padded by copies of the first and last frame of the video
            no_padded_frames_start = 25
            no_padded_frames_end = 25 + 50 - (len(frames) % 50 if len(frames) % 50 != 0 else 50)  # 25 - 74

            start_frame = np.expand_dims(frames[0], 0)
            end_frame = np.expand_dims(frames[-1], 0)
            padded_inputs = np.concatenate(
                [start_frame] * no_padded_frames_start + [frames] + [end_frame] * no_padded_frames_end, 0
            )

            ptr = 0
            while ptr + 100 <= len(padded_inputs):
                out = padded_inputs[ptr:ptr + 100]
                ptr += 50
                yield out

        res = []
        for inp in input_iterator():
            pred = self._predict_raw(np.expand_dims(inp, 0))[0, 25:75]
            res.append(pred)
            logger.debug(f"processed frames {min(len(res) * 50, len(frames))} / {len(frames)}")

        predictions = np.concatenate(res)[:len(frames)]  # remove extra padded frames
        scenes = get_scenes_from_predictions(predictions, self.threshold)

        save_path = os.path.join(save_dir, self.__name__ + ".scenes.txt")
        np.savetxt(save_path, scenes, "%d")

        visualize_predictions(frames, predictions, self.threshold).save(
            os.path.join(save_dir, self.__name__ + ".visualization.png"))

        return scenes

    def load_result_from_disk(self, save_dir: str):
        return np.genfromtxt(os.path.join(save_dir, self.__name__ + ".scenes.txt"), dtype=np.int32).reshape([-1, 2])
