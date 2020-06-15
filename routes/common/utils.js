
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
exports.getCsvChartData = function (dict) {
  const ents = Object.entries(dict);
  const len = Object.entries(dict).length;
  const numVals = Object.entries(dict)[0][1].length;

  csvString = "";
  // Header
  {
    let i = 0;
    for (const [key, testResults] of Object.entries(dict)) {
      csvString += key;

      if (i < len - 1) {
        csvString += ",\t\t";
      }
      ++i;
    }
  }
  csvString += "\n";

  // Data
  {
    for (let i = 0; i < numVals; ++i) {
      let j = 0;
      for (const [key, testResults] of Object.entries(dict)) {
        csvString += testResults[i];
        if (j < len - 1) {
          csvString += ",\t\t";
        }
        ++j;
      }
      csvString += "\n";
    }
  }
  return csvString;
};

/**
 * Takes JS dict variable and creates pretty formatted string representing the data in the CSV format.
 *
 *  INPUT FORMAT:
 *    dict = {
 *      x:  [
 *        [1,2],
 *        [1,2,3]
 *      ],
 *      fx: [
 *        [0.01, 0.04],
 *        [1.1, 0.14, 102.3]
 *      ]
 *    }
 *
 * OUTPUT CSV FORMAT:
 *    x,     len_2,   len_3
 *    1,     0.01,    1.1
 *    2,     0.04,    0.14
 *    3,     ,        102.3
 */
exports.getCsvMultiLineChartData = function (dict) {
  const numSeries = dict.fx.length;
  const yss = dict.fx;
  const xs = dict.x[dict.x.length - 1];

  let maxSessLen = 0;
  // Header
  csvString = "x";
  {
    for (let i = 0; i < numSeries; ++i) {
      const ys = yss[i];
      csvString += ", \t\t" + "length_" + String(ys.length);

      maxSessLen = Math.max(maxSessLen, ys.length);
    }
  }
  csvString += "\n";

  // Data
  for (let i = 0; i < maxSessLen; ++i) {
    // Add X
    csvString += xs[i];

    for (let j = 0; j < numSeries; ++j) {
      const y = typeof yss[j][i] !== "undefined" ? yss[j][i] : "";
      csvString += ",\t\t" + y;
    }
    csvString += "\n";
  }
  return csvString;
};

exports.setAnnotatorShowExamples = function (sess, newState) {
  sess.annotatorSettings.autocompleteWithExamples = newState;
};

exports.parseModelSettingsFromForm = function (sess, formBbody) {
  rankerUtils.checkOrInitSessionRankerObject(sess);

  // Get reference to current values
  const settings = sess.ranker.settings;

  // Number of results
  if (typeof formBbody.expansionSettigns !== "undefined") {
    settings.expansionSettigns = Number(formBbody.expansionSettigns);
  }

  //
  // If some are set, just overwrite them
  //

  // Number of results
  if (typeof formBbody.numResults !== "undefined") {
    settings.numResults = Number(formBbody.numResults);
  }
  // Keyword frequency settings
  if (typeof formBbody.keywordFrequency !== "undefined") {
    settings.keywordFrequency = Number(formBbody.keywordFrequency);
  }

  // Transformation
  {
    // Transformation ID
    if (typeof formBbody.aggregation !== "undefined") {
      settings.transformation.id = Number(formBbody.aggregation);
    }
    // Transformation parameter
    if (typeof formBbody.aggregationParameter !== "undefined") {
      settings.transformation.parameter = Number(formBbody.aggregationParameter);
    }
  }

  // Aggregation model
  {
    // Aggregation model ID
    if (typeof formBbody.rankingModel !== "undefined") {
      settings.aggregationModel.id = Number(formBbody.rankingModel);
    }

    // Boolean specific
    if (typeof formBbody.boolean_trueTreshold !== "undefined") {
      settings.aggregationModel.boolean.trueTreshold = Number(formBbody.boolean_trueTreshold);
    }
    if (typeof formBbody.boolean_inBucketRanking !== "undefined") {
      settings.aggregationModel.boolean.inBucketRanking = Number(formBbody.boolean_inBucketRanking);
    }

    // Viret specific
    if (typeof formBbody.viret_trueTreshold !== "undefined") {
      settings.aggregationModel.viret.trueTreshold = Number(formBbody.viret_trueTreshold);
    }
    if (typeof formBbody.viret_queryOperations !== "undefined") {
      settings.aggregationModel.viret.queryOperations = Number(formBbody.viret_queryOperations);
    }
  }

  // Model tests
  {
    if (typeof formBbody.dataSource !== "undefined") {
      settings.modelTests.dataSource = Number(formBbody.dataSource);
    }
  }

  // Simulated user
  {
    if (typeof formBbody.simulatedUser_exponent !== "undefined") {
      settings.simulatedUser.exponent = Number(formBbody.simulatedUser_exponent);
    }
  }

  return settings;
};

exports.convertSettingsObjectToNativeFormat = function (settings) {
  const result = {};

  result.numResults = Number(settings.numResults);
  result.keywordFrequency = Number(settings.keywordFrequency);

  result.dataSource = Number(settings.modelTests.dataSource);
  result.expansionSettigns = Number(settings.expansionSettigns);

  // =========================================
  // Simulated user settings

  result.simulatedUserSettings = [];

  // 0 => Simulated user exponent
  result.simulatedUserSettings.push(String(settings.simulatedUser.exponent));

  // ===========================================
  // Ranking model settings

  result.rankingModel = Number(settings.aggregationModel.id);
  result.rankingModelSettings = [];
  {
    switch (result.rankingModel) {
      // Boolean
      case 1:
        {
          // 0 =>
          result.rankingModelSettings.push(String(settings.keywordFrequency));

          // 1 ->
          result.rankingModelSettings.push(String(settings.aggregationModel.boolean.trueTreshold));

          // 2 =>
          result.rankingModelSettings.push(String(settings.aggregationModel.boolean.inBucketRanking));
        }
        break;

      // ViretBase
      case 3:
        {
          result.rankingModelSettings.push(String(settings.keywordFrequency));

          // 0 =>
          result.rankingModelSettings.push(String(settings.aggregationModel.viret.trueTreshold));

          // 1 =>
          result.rankingModelSettings.push(String(settings.aggregationModel.viret.queryOperations));
        }
        break;

      default:
        throw Error("Unknown model type.");
    }
  }

  //
  // Temporal queries setup
  //
  // Setup inner and outter temporal query operations
  if (
    settings.aggregationModel.viret.queryOperations == "2" ||
    settings.aggregationModel.viret.queryOperations == 2 ||
    settings.aggregationModel.viret.queryOperations == "3" ||
    settings.aggregationModel.viret.queryOperations == 3 ||
    settings.aggregationModel.id == "1" ||
    settings.aggregationModel.id == 1 // For Boolean
  ) {
    // Outter set to SUM
    result.rankingModelSettings.push(String("0"));
  } else {
    // Outter set to PRODUCT
    result.rankingModelSettings.push(String("1"));
  }

  // Inner always set to MAX
  result.rankingModelSettings.push(String("2"));

  result.aggregation = Number(settings.transformation.id);
  result.aggregationSettings = [];
  {
    // Variable with all settings
    result.aggregationSettings.push(String(settings.transformation.parameter));

    // If MAX inner operation
    if (
      settings.aggregationModel.viret.queryOperations == 1 ||
      settings.aggregationModel.viret.queryOperations == 3 ||
      settings.aggregationModel.viret.queryOperations == 4 ||
      settings.aggregationModel.boolean.inBucketRanking == 2
    ) {
      result.aggregationSettings.push(String("1"));
    } else {
      result.aggregationSettings.push(String("0"));
    }
  }
  return result;
};

exports.setUserLevel = function (sessionObject, level) {
  sess.userLevel = level;
};

exports.resolveUserLevel = function (req, viewData) {
  const sess = req.session;

  if (typeof sess.userLevel === "undefined") {
    // If loged as developer
    if (sess.isDev) {
      sess.userLevel = 10;
    } else {
      sess.userLevel = 1;
    }
  }

  viewData.userLevel = sess.userLevel;
  return;
};

exports.generateImageRankerConstructorArgs = function (inputImageSetIds, inputKeywordDataIds, inputScoringDataIds) {
  const params = [];

  for (let isId = 0; isId < inputImageSetIds.length; ++isId) {
    const paramsAA = [];
    const paramsAAw2v = [];
    const paramsA = [];
    const paramsB = [];
    const paramsC = [];

    // Path to images
    params.push(path.join(global.rootDir, global.cfg.imageSets[isId].imagesDir));

    // Get data dir path
    const isDir = path.join(global.rootDir, global.cfg.imageSets[isId].dataDir);

    for (let kwId = 0; kwId < inputKeywordDataIds.length; ++kwId) {
      const kwDir = path.join(isDir, global.cfg.imageSets[isId].keywordDataTypes[kwId].dataDir);

      paramsAA.push({
        keywordsDataType: global.cfg.imageSets[isId].keywordDataTypes[kwId].keywordType,
        filepath: path.join(kwDir, global.cfg.imageSets[isId].keywordDataTypes[kwId].keywordDataFilename),
      });

      if (global.cfg.imageSets[isId].keywordDataTypes[kwId].wordToVecFilename != "") {
        paramsAAw2v.push({
          keywordsDataType: global.cfg.imageSets[isId].keywordDataTypes[kwId].keywordType,
          filepath: path.join(kwDir, global.cfg.imageSets[isId].keywordDataTypes[kwId].wordToVecFilename),
        });
      } else {
        paramsAAw2v.push({
          keywordsDataType: global.cfg.imageSets[isId].keywordDataTypes[kwId].keywordType,
          filepath: "",
        });
      }

      for (let scId = 0; scId < inputScoringDataIds.length; ++scId) {
        const scDir = path.join(kwDir, global.cfg.imageSets[isId].keywordDataTypes[kwId].dataSets[scId].dataDir);

        paramsA.push({
          keywordsDataType: global.cfg.imageSets[isId].keywordDataTypes[kwId].keywordType,
          scoringDataType: global.cfg.imageSets[isId].keywordDataTypes[kwId].dataSets[scId].scoringDataType,
          filepath: path.join(
            scDir,
            global.cfg.imageSets[isId].keywordDataTypes[kwId].dataSets[scId].scoringDataFilename
          ),
        });

        const b = global.cfg.imageSets[isId].keywordDataTypes[kwId].dataSets[scId].softmaxScoringDataFilename;
        if (b != "") {
          paramsB.push({
            keywordsDataType: global.cfg.imageSets[isId].keywordDataTypes[kwId].keywordType,
            scoringDataType: global.cfg.imageSets[isId].keywordDataTypes[kwId].dataSets[scId].scoringDataType,
            filepath: path.join(
              scDir,
              global.cfg.imageSets[isId].keywordDataTypes[kwId].dataSets[scId].softmaxScoringDataFilename
            ),
          });
        }
        const c = global.cfg.imageSets[isId].keywordDataTypes[kwId].dataSets[scId].deepFeaturesFilename;
        if (c != "") {
          paramsC.push({
            keywordsDataType: global.cfg.imageSets[isId].keywordDataTypes[kwId].keywordType,
            scoringDataType: global.cfg.imageSets[isId].keywordDataTypes[kwId].dataSets[scId].scoringDataType,
            filepath: path.join(
              scDir,
              global.cfg.imageSets[isId].keywordDataTypes[kwId].dataSets[scId].deepFeaturesFilename
            ),
          });
        }

        paramsC.push();
      }
    }

    const aaa = path.join(
      global.rootDir,
      global.cfg.imageSets[isId].dataDir,
      global.cfg.imageSets[isId].keywordDataTypes[0].dataDir,
      global.cfg.imageSets[isId].keywordDataTypes[inputKeywordDataIds[0]].dataSets[inputKeywordDataIds[0]].dataDir
    );

    params.push(paramsAA);
    params.push(paramsA);
    params.push(paramsB);
    params.push(paramsC);
    params.push(
      path.join(
        aaa,
        global.cfg.imageSets[isId].keywordDataTypes[inputKeywordDataIds[0]].dataSets[inputKeywordDataIds[0]]
          .imageIdToFilename
      )
    );
    params.push(
      Number(
        global.cfg.imageSets[isId].keywordDataTypes[inputKeywordDataIds[0]].dataSets[inputKeywordDataIds[0]].idOffset
      )
    );
    params.push(Number(global.cfg.appMode));
    params.push(paramsAAw2v);

    // TODO
    break;
  }

  return params;
};
