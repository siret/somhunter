
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

// Require Lodash module
const moduleLodaSh = require("lodash");

// Get config file
const config = require("./config.json");

exports.initConfig = function() {
  // Default things are in development
  const defaultConfig = config.development;

  // Get current env setup
  const environment = process.env.NODE_ENV || "development";

  // Create enviroment config
  const environmentConfig = config[environment];

  // Merge to the final config
  const finalConfig = moduleLodaSh.merge(defaultConfig, environmentConfig);
  const cred = require("./user.json");

  // Login & auth credentials
  finalConfig.authPassword = cred.authPassword;
  finalConfig.authName = cred.authName;

  // Store final config in globals
  global.cfg = finalConfig;
};
