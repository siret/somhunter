import numpy as np
from PIL import Image, ImageDraw


def get_scenes_from_predictions(predictions: np.ndarray, threshold: float=0.1):
    predictions = (predictions > threshold).astype(np.uint8)

    scenes = []
    t, tp, start = -1, 0, 0
    for i, t in enumerate(predictions):
        if tp == 1 and t == 0:
            start = i
        if tp == 0 and t == 1 and i != 0:
            scenes.append([start, i])
        tp = t
    if t == 0:
        scenes.append([start, i])
    return np.array(scenes, dtype=np.int32)


def visualize_predictions(frames: np.ndarray, predictions: np.ndarray, threshold: float=0.1):
    nf, ih, iw, ic = frames.shape
    width = 25  # max(20, min(int(nf ** 0.5 / 2), 100))
    if len(frames) % width != 0:
        pad_with = width - len(frames) % width
        frames = np.concatenate([frames, np.zeros([pad_with, ih, iw, ic], np.uint8)])
        predictions = np.concatenate([predictions, np.zeros([pad_with], np.float32)])
    height = len(frames) // width

    scene = frames.reshape([height, width, ih, iw, ic])
    scene = np.concatenate(np.split(
        np.concatenate(np.split(scene, height), axis=2)[0], width
    ), axis=2)[0]

    img = Image.fromarray(scene)
    draw = ImageDraw.Draw(img)

    i = 0
    for h in range(height):
        for w in range(width):
            draw.line((w * iw + iw - 3, h * ih,
                       w * iw + iw - 3, (h + 1) * ih), fill=(0, 0, 0), width=4)
            draw.line((w * iw + iw - 3, h * ih + ih / 2 * (1 - predictions[i]),
                       w * iw + iw - 3, h * ih + ih / 2 * (1 + predictions[i])),
                      fill=(0, 255, 0) if predictions[i] > threshold else (255, 0, 0), width=2)
            draw.line((w * iw, h * ih, (w + 1) * iw, h * ih), fill=(255, 255, 255))
            i += 1
    return img
