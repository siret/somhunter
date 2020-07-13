#!/usr/bin/env python

import os
import glob
import yaml
import logging
import argparse

import scene_detection
import keyframe_selection
import feature_extraction
import thumbnail_extraction
from utils import ffmpeg_utils

os.environ["TF_CPP_MIN_LOG_LEVEL"] = "2"
logger = logging.getLogger(__file__)


def get_videos(sources):
    videos = []
    types = [".mp4", ".avi", ".mkv", ".webm", ".flv",  ".mpg", ".mov", ".m4v", ".mpe"
             ".mp2", ".mpe", ".mpv", ".mpeg", ".m2v"]
    for source in sources:
        source = os.path.normpath(source)
        if os.path.isfile(source):
            videos.append(source)
        elif os.path.isdir(source):
            videos.extend([fn for fn in glob.glob(os.path.join(source, "*"))
                           if os.path.isfile(fn)])
        else:
            videos.extend([fn for fn in glob.glob(source) if os.path.isfile(fn)])
    for video in videos:
        if len([ext for ext in types if video.endswith(ext)]) == 0:
            print("ERROR: File {} is probably not video file.".format(video))
            print("       You can add its extension into code manually if you are sure it is a video file.")
            exit(1)
    return sorted(videos)


def get_target_dir(store_to, force=False):
    store_to = os.path.normpath(store_to)
    if os.path.exists(store_to):
        if not force:
            print("ERROR: Directory {} already exists.".format(store_to))
            print("       Run script with `--force` to rewrite already existing directory.")
            exit(1)
    else:
        os.makedirs(store_to)
    return store_to


class Processor:

    def __init__(self, config, save_dir, dataset_name):
        with open(config, "r") as f:
            self._config = yaml.load(f, Loader=yaml.Loader)

        self._scene_det = scene_detection.TransNetV1(
            proto_file=self._config["transnet"]["pb_file"])
        self._kf_select = keyframe_selection.KeyFrameSelector(
            merge_threshold=self._config["keyframes"]["merge_threshold"],
            min_frames_in_cluster=self._config["keyframes"]["min_frames_in_cluster"],
            max_frames_in_cluster=self._config["keyframes"]["max_frames_in_cluster"],
            interpolation_bound=self._config["keyframes"]["interpolation_bound"],
            visualize=self._config["keyframes"]["visualize"])
        self._w2vv_extr = feature_extraction.W2VVFeatureExtractor(
            networks=self._config["w2vv"]["networks"],
            image_weight_matrix=self._config["w2vv"]["image_weight_matrix"],
            image_bias_vector=self._config["w2vv"]["image_bias_vector"],
            n_components=self._config["w2vv"]["n_components"]
        )
        self._thum_extr = thumbnail_extraction.ThumbnailExtractor(
            quality=self._config["images"]["quality"]
        )

        self.save_dir = save_dir
        self.dataset_name = dataset_name

    def run_on_video(self, filename, video_save_dir, video_id):
        os.makedirs(video_save_dir, exist_ok=True)

        logger.info(f"extracting all video frames in resolution 48x27")
        frames = ffmpeg_utils.extract_all_frames(filename, output_width=48, output_height=27)
        logger.info(f"predicting scenes")
        scenes = self._scene_det.predict_and_save(frames, video_save_dir)
        logger.info(f"selecting key frames")
        keyframe_indices = self._kf_select.predict_and_save(frames, scenes, video_save_dir)

        logger.info(f"extracting selected video frames in resolution 224x224")
        selected_frames = ffmpeg_utils.extract_selected_frames(
            filename, keyframe_indices, output_width=224, output_height=224)
        logger.info(f"extracting key frame features")
        self._w2vv_extr.predict_and_save(selected_frames, video_save_dir)

        logger.info(f"extracting selected video frames in resolution "
                    f"{self._config['images']['width']}x{self._config['images']['height']}")
        selected_frames = ffmpeg_utils.extract_selected_frames(
            filename,
            keyframe_indices,
            output_width=self._config["images"]["width"],
            output_height=self._config["images"]["height"])
        logger.info(f"saving key frames as jpg files")
        self._thum_extr.predict_and_save(selected_frames, keyframe_indices, scenes, video_id, video_save_dir)

        return len(keyframe_indices)

    def run(self, filenames):
        video_lengths, video_dirs = [], []
        logger.warning(f"running extraction on {len(filenames)} video files")

        for video_id, filename in enumerate(filenames):
            video_save_dir = os.path.join(self.save_dir, os.path.splitext(os.path.basename(filename))[0])
            video_dirs.append(video_save_dir)

            logger.warning(f"processing {filename} to {video_save_dir} ({video_id + 1}/{len(filenames)})")
            n_keyframes = self.run_on_video(filename, video_save_dir, video_id)
            video_lengths.append(n_keyframes)

        logger.warning(f"merging extracted data into one index")
        self._w2vv_extr.merge(video_dirs, video_lengths, os.path.join(self.save_dir, self.dataset_name))
        self._thum_extr.merge(video_dirs, video_lengths, os.path.join(self.save_dir, self.dataset_name))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="extraction-pipeline")
    parser.add_argument("config", type=str, help="path to a config file")
    parser.add_argument("store_to", type=str, help="path to a directory where to store the dataset")
    parser.add_argument("sources", type=str, nargs="+", help="list of files or directories with video files")
    parser.add_argument("--force", action="store_true", help="rewrite already existing directory")
    parser.add_argument("--name", type=str, help="dataset name", default="extracted_dataset")
    args = parser.parse_args()

    logging.basicConfig(level=logging.INFO,
                        format="<%(asctime)s> %(levelname)s %(name)s - %(message)s",
                        datefmt="%H:%M:%S")

    videos = get_videos(args.sources)
    store_to = get_target_dir(args.store_to, args.force)

    processor = Processor(args.config, store_to, args.name)
    processor.run(videos)
