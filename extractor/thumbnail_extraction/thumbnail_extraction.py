import os
import numpy as np
from PIL import Image


class ThumbnailExtractor:

    def __init__(self, quality, name="thumbnails"):
        self.quality = quality
        self.__name__ = name

    def predict_and_save(self, selected_frames: np.ndarray, keyframe_indices: np.ndarray, scenes: np.ndarray,
                         video_id: int, save_dir: str):
        assert selected_frames.dtype == np.uint8 and len(selected_frames.shape) == 4, \
            "Input shape of `selected_frames` must be [frames, height, width, 3] and type np.uint8."
        assert len(selected_frames) == len(keyframe_indices), \
            "Length of `selected_frames` must be the same as `keyframe_indices`."
        os.makedirs(os.path.join(save_dir, "images"), exist_ok=True)

        with open(os.path.join(save_dir, self.__name__ + ".dataset.txt"), "w") as f:
            kf_idx = 0
            for scene_no, (start, end) in enumerate(scenes):
                assert kf_idx == len(keyframe_indices) or keyframe_indices[kf_idx] >= start
                while kf_idx < len(keyframe_indices) and keyframe_indices[kf_idx] <= end:
                    filename = "{:05d}/v{:05d}_s{:05d}(f{:08d}-f{:08d})_f{:08d}.jpg".format(
                        video_id, video_id, scene_no, start, end, keyframe_indices[kf_idx])
                    f.write(filename + "\n")

                    image = selected_frames[kf_idx]
                    image = Image.fromarray(image)
                    image.save(os.path.join(save_dir, "images", filename), format="jpeg", quality=self.quality)

                    kf_idx += 1
            assert kf_idx == len(keyframe_indices)

    def load_result_from_disk(self, save_dir: str):
        with open(os.path.join(save_dir, self.__name__ + ".dataset.txt"), "r") as f:
            return f.read()

    def merge(self, videos, no_frames_per_video, save_file):
        with open(save_file + ".dataset", "w") as f:
            for v in videos:
                f.write(self.load_result_from_disk(v))
