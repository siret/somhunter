import cv2
import numpy as np
from PIL import Image, ImageDraw


def compute_color_histograms(frames: np.ndarray) -> np.ndarray:
    assert list(frames.shape[1:]) == [27, 48, 3], "compute_color_histograms works only with frames of shape [27, 48, 3]"
    histograms = np.empty([len(frames), 12, 6, 6, 6])

    for i in range(len(frames)):
        j = 0
        for y_low, y_high in [(0, 9), (9, 18), (18, 27)]:
            for x_low, x_high in [(0, 12), (12, 24), (24, 36), (36, 48)]:
                histograms[i, j] = cv2.calcHist([frames[i, y_low:y_high, x_low:x_high]],
                                                [0, 1, 2], None, [6, 6, 6], [0, 256, 0, 256, 0, 256])
                j += 1
    return histograms


def visualize_scenes_and_keyframes(frames: np.ndarray, scenes: np.ndarray, keyframes: np.ndarray, clusters: np.ndarray):
    nf, ih, iw, ic = frames.shape
    width = 25
    if len(frames) % width != 0:
        pad_with = width - len(frames) % width
        frames = np.concatenate([frames, np.zeros([pad_with, ih, iw, ic], np.uint8)])
    height = len(frames) // width

    scene = frames.reshape([height, width, ih, iw, ic])
    scene = np.concatenate(np.split(
        np.concatenate(np.split(scene, height), axis=2)[0], width
    ), axis=2)[0]

    img = Image.fromarray(scene)
    draw = ImageDraw.Draw(img, "RGBA")

    def draw_start_frame(frame_no):
        w = frame_no % width
        h = frame_no // width
        draw.rectangle([(w * iw, h * ih), (w * iw + 2, h * ih + ih - 1)], fill=(255, 0, 0))
        draw.polygon(
            [(w * iw + 7, h * ih + ih // 2 - 4), (w * iw + 12, h * ih + ih // 2), (w * iw + 7, h * ih + ih // 2 + 4)],
            fill=(255, 0, 0))
        draw.rectangle([(w * iw, h * ih + ih // 2 - 1), (w * iw + 7, h * ih + ih // 2 + 1)], fill=(255, 0, 0))

    def draw_end_frame(frame_no):
        w = frame_no % width
        h = frame_no // width
        draw.rectangle([(w * iw + iw - 1, h * ih), (w * iw + iw - 3, h * ih + ih - 1)], fill=(255, 0, 0))
        draw.polygon([(w * iw + iw - 8, h * ih + ih // 2 - 4), (w * iw + iw - 13, h * ih + ih // 2),
                      (w * iw + iw - 8, h * ih + ih // 2 + 4)], fill=(255, 0, 0))
        draw.rectangle([(w * iw + iw - 1, h * ih + ih // 2 - 1), (w * iw + iw - 8, h * ih + ih // 2 + 1)],
                       fill=(255, 0, 0))

    def draw_transition_frame(frame_no):
        w = frame_no % width
        h = frame_no // width
        draw.rectangle([(w * iw, h * ih), (w * iw + iw - 1, h * ih + ih - 1)], fill=(128, 128, 128, 180))

    def draw_key_frame(frame_no):
        w = frame_no % width
        h = frame_no // width
        draw.rectangle([(w * iw, h * ih), (w * iw + iw - 1, h * ih + ih - 1)], outline=(255, 255, 0))

    def draw_cluster_split(frame_no):
        w = frame_no % width
        h = frame_no // width
        draw.polygon(
            [(w * iw - .5, h * ih + ih // 2 - 6), (w * iw + 5, h * ih + ih // 2), (w * iw - .5, h * ih + ih // 2 + 6),
             (w * iw - 7, h * ih + ih // 2)], fill=(0, 255, 0), outline=(0, 0, 0))

    curr_frm, curr_scn, curr_kfm, curr_ctr = 0, 0, 0, 0

    while curr_scn < len(scenes):
        start, end = scenes[curr_scn]
        # gray out frames that are not in any scene
        while curr_frm < start:
            draw_transition_frame(curr_frm)
            curr_frm += 1

        # draw start and end of a scene
        draw_start_frame(curr_frm)
        draw_end_frame(end)

        # draw all keyframes in the scene
        while curr_kfm < len(keyframes) and keyframes[curr_kfm] <= end:
            draw_key_frame(keyframes[curr_kfm])
            curr_kfm += 1

        # draw all cluster splits in the scene
        while curr_ctr < len(clusters) and clusters[curr_ctr] <= end:
            draw_cluster_split(clusters[curr_ctr])
            curr_ctr += 1

        # go to the next scene
        curr_frm = end + 1
        curr_scn += 1

    # gray out the last frames that are not in any scene (if any)
    while curr_frm < nf:
        draw_transition_frame(curr_frm)
        curr_frm += 1

    return img
