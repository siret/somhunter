import numpy as np
import os
from shutil import copyfile
import argparse
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument("--in_root", default=".", type=str, help="Root of input files")
parser.add_argument("--out_root", default="./output", type=str, help="Root of output files")
parser.add_argument("--nth", default=10, type=int, help="Take every nth")



def filter_dataset_keyframes(_in_path, _out_path, _args):
    new_dataset = []
    count = 0
    with open(_in_path, 'r') as in_file:
        with open(_out_path, 'w') as out_file:
            all_lines = in_file.readlines()
            count = len(all_lines)
            for index, line in enumerate(all_lines):
                if (index % _args.nth) == 0:
                    new_dataset.append(line)
                    out_file.write(line)
    return new_dataset, count


def filter_features(_in_path, _out_path, _args, _n, _dim):
    features = np.fromfile(_in_path, dtype=np.float32)
    features = features[3:].reshape((_n, _dim))
    new_features = np.zeros(shape=((int(_n / _args.nth) + 1) * _dim + 3),dtype=np.float32)
    for new_index, old_index in enumerate(range(0, _n, _args.nth)):
        new_offset = 3 + new_index * _dim
        if (_n - old_index) < 200:
            print(old_index)
        new_features[new_offset : new_offset + _dim] = features[old_index]
    
    with open(_out_path, 'wb') as out_f:
        np.save(out_f, new_features)
    return new_features

def filter_thumbs(_in_path, _out_path, _args, _new_dataset):
    for thumb in _new_dataset:
        vid_dir = thumb.split('/')[0]
        thumb = thumb[:-1]
        if not os.path.exists(os.path.join(_out_path, vid_dir)):
            os.makedirs(os.path.join(_out_path, vid_dir))
        copyfile(os.path.join(_in_path, thumb), os.path.join(_out_path, thumb))


if __name__ == "__main__":
    args = parser.parse_args()
    Path(args.out_root).mkdir(parents=True, exist_ok=True)
    new_dataset, count = filter_dataset_keyframes(os.path.join(args.in_root, "V3C1_20191215.keyframes.dataset"), os.path.join(args.out_root, "V3C1_20191215.keyframes.dataset"), args)
    new_features = filter_features(os.path.join(args.in_root, "V3C1_20191228.w2vv.images.normed.128pca.viretfromat"), 
                                    os.path.join(args.out_root, "V3C1_20191228.w2vv.images.normed.128pca.viretfromat"), 
                                    args, count, 128)
    filter_thumbs(os.path.join(args.in_root, "thumbs"), os.path.join(args.out_root, "thumbs"), args, new_dataset)
