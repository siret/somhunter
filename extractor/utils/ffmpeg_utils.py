import ffmpeg
import subprocess
import numpy as np


# hack for steam.run_async(quiet=True) bug
def _run_async_quiet(stream_spec):
    args = ffmpeg._run.compile(stream_spec, "ffmpeg", overwrite_output=False)
    return subprocess.Popen(
        args, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL
    )


def extract_selected_frames(video_filename, selected_frames, output_width, output_height):
    video_stream = _run_async_quiet(
        ffmpeg.input(video_filename).output("pipe:", format="rawvideo", pix_fmt="rgb24",
                                            s="{}x{}".format(output_width, output_height))
    )

    out_frames = []
    idx, frame_no = 0, -1

    while True:
        frame_no += 1

        in_bytes = video_stream.stdout.read(output_width * output_height * 3)
        if not in_bytes:
            raise Exception("`selected_frames` contain indexes larger than video length.")

        if frame_no != selected_frames[idx]:
            continue

        frame = np.frombuffer(in_bytes, np.uint8).reshape([output_height, output_width, 3])
        out_frames.append(frame)

        idx += 1
        if idx == len(selected_frames):
            break

    return np.stack(out_frames)


def extract_all_frames(video_filename, output_width, output_height):
    video_stream, err = (
        ffmpeg
        .input(video_filename)
        .output("pipe:", format="rawvideo", pix_fmt="rgb24", s="{}x{}".format(output_width, output_height))
        .run(capture_stdout=True, capture_stderr=True)
    )

    return np.frombuffer(video_stream, np.uint8).reshape([-1, output_height, output_width, 3])
