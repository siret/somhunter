
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
"use strict";

const sessState = require("./SessionState");

exports.initRequest = function (req, viewData, routeSettings) {
  // \todo Remove this, it's just for the dev. xoxo
  //req.session.state = null;

  return {};
};

exports.genericPreProcessReq = function (req, viewData, routeSettings) {
  this.checkGlobalSessionState(req, viewData);

  // Get current page slug
  viewData.currentPage = routeSettings.slug;
};

exports.genericProcessReq = function (req, viewData, routeSettings) {};

exports.genericPostProcessReq = function (req, viewData, routeSettings) {
  this.checkGlobalViewState(req, viewData);
};

exports.checkGlobalSessionState = function (req, viewData) {
  const sess = req.session;

  if (typeof sess.state === "undefined" || sess.state == null) {
    // xoxo
    // Construct the session state here
    sess.state = sessState.Construct();
  }
  // At this point sess.state should be always populated with correct values
};

exports.checkGlobalViewState = function (req, viewData) {
  const sess = req.session;

  // Initialize viewData
  viewData.somhunter = {};
};
