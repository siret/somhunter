

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

const createError = require("http-errors");
const express = require("express");
const session = require("express-session");
const path = require("path");
const cookieParser = require("cookie-parser");
const morgan = require("morgan");
const fs = require("fs");

const { createLogger, format, transports } = require("winston");
const { combine, timestamp, printf } = format;

/*
 * Configuration
 */

// Set root project directory
global.rootDir = __dirname;

// Load config
const config = require("./config/config");
config.initConfig();

/*
 * Request routing
 */
// Page routers
const somhunterRouter = require("./routes/somhunter");
const routerNotFound = require("./routes/404");

// API endpoints
const endpoints = require("./routes/endpoints");

/*
 * Logging
 */
// Log format closure
const myFormat = printf(({ level, message, timestamp }) => {
  return `${timestamp} ${level}: ${message}`;
});

// Make sure that this dir exists
fs.mkdir(global.cfg.logsDir, { recursive: true }, (err) => {
  if (err) throw err;
});

// Create main logger
global.logger = createLogger({
  level: "debug",
  format: combine(timestamp(), myFormat),
  defaultMeta: { service: "user-service" },
  transports: [
    // Write all logs with level `error` and below to `error.log`
    // Write all logs with level `info` and below to `combined.log`
    new transports.Stream({
      stream: fs.createWriteStream(global.cfg.logsDir + "error.log", { flags: "a+" }),
      level: "error",
    }),
    new transports.Stream({ stream: fs.createWriteStream(global.cfg.logsDir + "combined.log", { flags: "a+" }) }),
  ],
});

// Log only into log files while in production
if (process.env.NODE_ENV !== "production") {
  global.logger.add(
    new transports.Console({
      format: combine(timestamp(), myFormat),
    })
  );
}

/*
 * Launch the app
 */
// Instantiate Express app
const app = express();

// Setup where EJS templates are stored
app.set("views", path.join(__dirname, "views"));

// Setup EJS engine for templates
app.set("view engine", "ejs");
app.use(express.json());
app.use(express.urlencoded({ extended: false }));
app.use(cookieParser());
app.use(express.static(path.join(__dirname, "public")));

// Use Morgan for request logging
app.use(
  morgan("common", {
    stream: fs.createWriteStream(__dirname + "/" + global.cfg.logsDir + "/requests.log", { flags: "a+" }),
  })
);
app.use(morgan("dev"));

/*
 * HTTP authentication
 */
app.use((req, res, next) => {
  // Get auth credentials
  const auth = {
    login: global.cfg.authName,
    password: global.cfg.authPassword,
  };

  // Parse login and password from headers
  const b64auth = (req.headers.authorization || "").split(" ")[1] || "";
  const [login, password] = new Buffer(b64auth, "base64").toString().split(":");

  // Verify login and password are set and correct
  if (login && password && login === auth.login && password === auth.password) {
    // Access granted
    return next();
  }

  // Access denied
  res.set("WWW-Authenticate", "Basic realm='401'");
  res.status(401).send("Authentication required.");
});

/*
 * Turn on sessions
 */
app.use(session({ secret: "matfyz", resave: false, saveUninitialized: true }));

/*
 * Initial communication with the admin user
 */
global.logger.log("debug", "process.env = '" + process.env["NODE_ENV"] + "'");
if (process.env["NODE_ENV"] === "development") {
  global.logger.log(
    "warn",
    `
  =======================================================================
  App is running in 'development' environment!
  
  Debug info will be printed. 
  This is not recommended while exposed to public users.
  
  To change it, you can set environment variable \`NODE_ENV\` to 'production.

  =======================================================================`
  );
}

/*
 * Load native evaluation library (ImageRanker)
 */
const dataConfigFilepath = path.join(global.rootDir, global.cfg.dataConfigFilepath);

global.logger.log("info", "Initializing SOMHunter core...");
global.logger.log("debug", "dataConfigFilepath = " + dataConfigFilepath);

// Create global instance of the ImageRanker
const core = require(path.join(__dirname, "build/Release/somhunter_core.node"));
global.core = new core.SomHunterNapi(dataConfigFilepath);
global.logger.log("info", "SOMHunter is ready...");

/*
 * Push all routers into express middleware stack
 */
app.use("/", somhunterRouter);

// SOMHunter endpoints
app.get("/get_frame_detail_data", endpoints.getFrameDetailData);
app.get("/get_autocomplete_results", endpoints.getAutocompleteResults);
app.get("/get_top_screen", endpoints.getTopScreen);
app.get("/get_som_screen", endpoints.getSomScreen);
app.post("/submit_frame", endpoints.submitFrame);
app.post("/reset_search_session", endpoints.resetSearchSession);

app.post("/rescore", endpoints.rescore);
app.post("/like_frame", endpoints.likeFrame);
app.post("/unlike_frame", endpoints.unlikeFrame);

// 404 fallback
app.use("/404", routerNotFound);

// Error handler
app.use(function (err, req, res, next) {
  // Show error message if in development environment
  res.locals.message = err.message;
  res.locals.error = req.app.get("env") === "development" ? err : {};

  // render the error page
  res.status(err.status || 500);
  res.render("error");
});

module.exports = app;
