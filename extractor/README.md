# SOMHunter data extractor

This extracts frames from various video files, and creates thumbnails and metadata that can be used to search through the video in SOMHunter.

The process is, roughly, as follows:
1. download the required pre-trained neural nets
2. build a docker container that contains the environment necessary for the extractor to work
3. convert a bunch of video files into a SOMHunter dataset
4. move the extracted dataset to SOMHunter

If you use the extractor for commercial or publishing purposes, you should cite (or otherwise credit) the authors of the corresponding software:

- Lokoč, J., Kovalčik, G., Souček, T., Moravec, J., & Čech, P.
  (2019, October).
  [A framework for effective known-item search in video](https://dl.acm.org/doi/abs/10.1145/3343031.3351046).
  In *Proceedings of the 27th ACM International Conference on Multimedia* (pp. 1777-1785).
- Li, X., Xu, C., Yang, G., Chen, Z., & Dong, J.
  (2019, October).
  [W2VV++ Fully Deep Learning for Ad-hoc Video Search](https://dl.acm.org/doi/abs/10.1145/3343031.3350906).
  In *Proceedings of the 27th ACM International Conference on Multimedia* (pp. 1786-1794).
- Mettes, P., Koelma, D. C., & Snoek, C. G. (2020).
  [Shuffled ImageNet Banks for Video Event Detection and Search](https://dl.acm.org/doi/abs/10.1145/3377875).
  ACM *Transactions on Multimedia Computing, Communications, and Applications* (TOMM), 16(2), 1-21.
- The ResNet152 model from [Apache MXNet](https://mxnet.incubator.apache.org/) framework

## 1. Downloading the neural networks

First, download ResNext101 and ResNet152 into the `models` directory:

```sh
( cd models
wget http://data.mxnet.io/models/imagenet-11k/resnet-152/resnet-152-0000.params
wget http://data.mxnet.io/models/imagenet-11k/resnet-152/resnet-152-symbol.json
wget https://isis-data.science.uva.nl/mettes/imagenet-shuffle/mxnet/resnext101_bottomup_12988/resnext-101-1-0040.params
wget https://isis-data.science.uva.nl/mettes/imagenet-shuffle/mxnet/resnext101_bottomup_12988/resnext-101-symbol.json
)
```

The first 2 files are not available via https; you may still check their checksums to verify that they match the expectations:

```
 $ sha256sum resnet-152-*
a9836894d16ecebbf4b6e3fa7166711c61027db3f7adf01036ca474fb00ef4f2  resnet-152-0000.params
58be9bc993124abefde86c07914edd6202d550b3308b4270d3028c08b1c5e187  resnet-152-symbol.json
```

If required, edit the `config.yaml` file to change the paths to the downloaded models.


## 2. Building the docker container

We assume there is a CUDA-capable card available for the computation. (If not, it is possible to modify the scripts to use much slower CPU computation.) Use `nvidia-docker` as available from https://github.com/NVIDIA/nvidia-docker .

With the docker installed, you should be able to build the container by running the following command in the extractor directory (i.e. next to the `Dockerfile`):
```sh
docker build -t somhunter/extractor .
```

## 3. Extrating the data

After the container is built, you can run it as follows:

```sh
docker run
    --gpus 1
    --rm -ti
    -v /PATH/TO/YOUR/VIDEOS:/videos
    -v $PWD:/workdir
    somhunter/extractor
    workdir/export.py config.yaml /videos/output /videos/*.mp4
    --name MyDataset
```

In the command, change the paths so that `/PATH/TO/YOUR/VIDEOS` is a directory that contains your video data (preferably `mp4` files). If you run the container from a different directory than from extractor, you also need to change the `$PWD` path accordingly.

The dataset name `MyDataset` serves for naming the files correctly (you can rename the files later).

The extraction output should appear in the directory `/path/to/videos/output`.

## 4. Using the data in SOMHunter

The output directory should now contain the following files:

- `MyDataset.w2vv.normed.128pca.viretformat`
- `MyDataset.w2vv.pca.matrix.bin`
- `MyDataset.w2vv.pca.mean.bin`
- `MyDataset.dataset`

Move these to the `data` directory in SOMHunter and change the paths in `config.json` that originally point to the `ITEC` dataset to point to your new `MyDataset`.

Finally, lots of thumbnails are generated in subdirectories, with names like `video_name/images/v00000_s00000(f00000000-f00000000)_f00000000.jpg`. You should verify that the extraction matches your expectations. After that, it is necessary to move the thumbnails to the places where SOMHunter expects them, which can be done with a bit of shell scripting, as follows:

```sh
find "/PATH/TO/YOUR/VIDEOS/output/" -name 'v*_s*_f*.jpg' | while read path ; do
  fn=`basename "$path"`
  vid="${fn%%_s*}"
  vid="${vid#v}"
  dirname="/PATH/TO/SOMHUNTER/public/thumbs/$vid"
  mkdir -v -p "$dirname"
  cp -v "$path" "$dirname"
done
```

Replace the paths `/PATH/TO/...` as required for your setup.

The following extra files are necessary for proper loading of the datasets, but you can re-use the ones supplied with the ITEC dataset as their content is the same for all datasets:
- `txt_bias-2048floats.bin`
- `txt_weight-11147x2048floats.bin`
- `word2idx.txt`
