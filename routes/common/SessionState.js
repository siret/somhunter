
/* This file is part of SOMHunter.
 *
 * Copyright (C) 2020 František Mejzlík <frankmejzlik@gmail.com>
 *                    Mirek Kratochvil <exa.exa@gmail.com>
 *                    Patrik Veselý <prtrikvesely@gmail.com>
 *
 * SOMHunter is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 2 of the License, or (at your option)
 * any later version.
 *
 * SOMHunter is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * SOMHunter. If not, see <https://www.gnu.org/licenses/>.
 */
const { resetSearchSession } = require("../endpoints");

exports.Construct = function () {
  return {
    textQueries: {
      q0: { value: "" },
      q1: { value: "" },
    },
    likes: [],
    unlikes: [],
    frameContext: {
      frameId: null,
      frames: [],
    },
    screen: null,
  };
};

exports.setTextQueries = function (state, q0, q1) {
  state.textQueries.q0.value = q0;
  state.textQueries.q1.value = q1;
};

exports.switchScreenTo = function (state, screen, frames, targetFrame) {
  // Apply current likes
  for (let i = 0; i < frames.length; ++i) {
    // If UI has it liked
    if (state.likes.includes(frames[i].id)) {
      frames[i].liked = true;
    }

    // If null image
    if (frames[i].id == null) {
      frames[i].src = "/images/no_img.jpg";
    }
  }

  state.screen = {
    type: screen,
    frames: frames,
  };
  
  state.frameContext.frameId = targetFrame;
};

exports.resetSearchSession = function (state) {
  state.textQueries = {
    q0: { value: "" },
    q1: { value: "" },
  };
  (state.likes = []),
    (state.unlikes = []),
    (state.frameContext = {
      frameId: null,
      frames: [],
    });
  state.screen = null;
};

exports.getLikes = function (state) {
  return state.likes;
};

exports.getUnlikes = function (state) {
  return state.unlikes;
};

exports.resetLikes = function (state) {
  state.likes = [];

  const frames = state.screen.frames;
    for (let i = 0; i < frames.length; ++i) {
      frames[i].liked = false;
    }
};

exports.resetUnlikes = function (state) {
  state.unlikes = [];
};

exports.getSomhunterUiState = function (state) {
  // For now it's just the whole state
  return state;
};

exports.likeFrame = function (state, frameId) {
  // Make sure that it's not in unliked
  // Those two sets must be disjunctive
  state.unlikes.filter((x) => x !== frameId);

  // Add it to the likes
  state.likes.push(frameId);

  // Walk all the frames and like it
  const frames = state.screen.frames;
  for (let i = 0; i < frames.length; ++i) {
    if (frames[i].id == frameId) {
      frames[i].liked = true;
    }
  }

  return true;
};

exports.unlikeFrame = function (state, frameId) {
  // Make sure it's liked in the first place
  if (state.likes.includes(frameId)) {
    state.likes.filter((x) => x !== frameId);
    state.unlikes.push(frameId);

    // Walk all the frames and unlike it
    const frames = state.screen.frames;
    for (let i = 0; i < frames.length; ++i) {
      if (frames[i].id == frameId) {
        frames[i].liked = false;
      }
    }

    return true;
  }
  return false;
};
