
# HOW-TO: Adding a new rescore function

This tutorial briefs the implementation and modification of the existing frame
scoring functions.

Rescore functions are implemented in the backend, currently in the module
`RelevanceScore`. In the current version, there is only `apply_bayes` function
(in `core/src/RelevanceScores.cpp`). Several additional functions serve for
initial ranking of the frames with keyword-based scores, these can be found in
the `KeywordRanker` module.

The current Bayesian ranker uses almost exclusively the data available from
`DatasetFeatures`, i.e. the feature vectors for each frame; these are
"compared" using a distance measure based on cosine simiarity.

We show how to construct a very simple rescoring function that modifies the
score based on sum of distances from all user-marked frames.

## 1. Write the re-scorer

The re-scoring function can be added to the ScoreModel for simplicity. Add a
method header in `core/src/RelevanceScore.h` as such:
```cpp
void apply_simple(const std::set<ImageId>& likes, const DatasetFeatures &features);
```

The implementation goes preferentially into `core/src/RelevanceScore.cpp`:
```cpp
void
ScoreModel::apply_simple(const std::set<ImageId>& likes,
                        const DatasetFeatures &features)
{
	for (ImageId ii = 0; ii < scores.size(); ++ii) {
		float sum = 0;

		for (ImageId i : likes)
			sum += features.d_dot(ii, i);

		scores[ii] *= pow(0.5, -sum);
	}

  	normalize();
}
```

The `normalize()` call at the end keeps the scores in a relatively sane
interval, and prevents various numeric problems. `features` are used as a main
data source.

## 2. Plug the new function into the rescorer
Relevance scores are updated everytime `rescore()` is called.

We will modify `rescore` in `core/src/SomHunter.cpp` to call the customized
function as follows:
```cpp
void
SomHunter::rescore(std::string text_query)
{
	submitter.poll();

	// Rescore text query
	rescore_keywords(std::move(text_query));

	// Rescore relevance feedback
	if (!likes.empty())
	{
		scores.apply_simple(likes, features);
	}

	// Start SOM computation
	som_start();

	// Update search context
	shown_images.clear();

	// Reset likes
	likes.clear();
	for (auto &fr : frames) {
		fr.liked = false;
	}

	auto top_n = scores.top_n(frames,
	                          TOPN_LIMIT,
	                          config.topn_frames_per_video,
	                          config.topn_frames_per_shot);
                            
	// logging (see the description below)
	submitter.submit_and_log_rescore(frames,
	                                 scores,
	                                 used_tools,
	                                 current_display_type,
	                                 top_n,
	                                 last_text_query,
	                                 config.topn_frames_per_video,
	                                 config.topn_frames_per_shot);
}
```

## 3. Optional: Pass additional information to the rescorer from the front-end

If your rescoring method needs more information than plain likes and dislikes,
you need to modify the function that pass the data to the backend, namely:

- backend rescorer interface in `SomHunter.h`
- backend N-API wrapper in `SomHunterNapi.cpp` and `.h`
- route handler in `routes`
- optionally, add the controls for the new functionality to `routes` and `views`.

Use the [N-API documentation](https://nodejs.org/api/n-api.html) to get an
overview of possiblilities of data transfer to frontend. For a similar
modification, you can look at the code in [the tutorial about new display
types](HOWTO-display.md).

## 4. Logging

Finally, VBS rules say that all user actions must be logged with all relevant
parameters, to aid future analysis!

Various logging functions are available in `Submitter.h` and `Submitter.cpp`,
and it's generally easy to create various customized ones. VBS defines message
categories and types which should be filled in properly; the "contents" of the
log message is otherwise an arbitrary string, but should be
machine-interpretable.

Notably, VBS rules may change (we may update this repository accordingly).

The actual rescoring function calls the correct submitter method with many
parameters that should be logged:
```cpp
submitter.submit_and_log_rescore(frames, // frames marked by the user
				 scores, // instance of scores used for collecting additional data
				 used_tools, // a simple list of tools already used for this search (class UsedTools)
				 current_display_type, // what display the user employed to submit the feedback
				 top_n, // currently top-scoring frames
				 last_text_query, // last text query
				 config.topn_frames_per_video, // current framefilter settings
				 config.topn_frames_per_shot);
```

If required, you may supply your own information; use the function `push_event`
in `Submitter` that fills the log message into the "backlog" of events that are
sent periodically:
```cpp
void push_event(const std::string &cat,
		const std::string &type,
		const std::string &value);
```

Or, start the sending with any custom contents immediately, as in the
`submit_and_log_rescore` function:

```cpp
//...

Json top = Json::object{ { "teamId", int(cfg.team_ID) },
			 { "memberId", int(cfg.member_ID) },
			 { "timestamp", double(timestamp()) },
			 { "usedCategories", used_cats },
			 { "usedTypes", used_types },
			 { "sortType", sort_types },
			 { "resultSetAvailability", "top" },
			 { "type", "result" },
			 { "value", query_val },
			 { "results", std::move(result_json_arr) } };

start_sender(cfg.submit_rerank_URL, "", top.dump());
```

