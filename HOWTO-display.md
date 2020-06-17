
# HOW-TO: Adding a new display type

This tutorial shows the implementation of a simple display variant (a k-NN display), as an basic example of adding UI functionality. In particular, we show how to

- create the backend plumbing required to generate the data for the display
- create a new display on the frontend
- connect both parts using N-API

## 1. Add backend functionality

You can add any required API functions to `core/src/SomHunter.h`. For display purposes, the frontend calls this method:
```cpp
FramePointerRange get_display(DisplayType d_type, ImageId selected_image = 0, PageId page = 0);
```

A possible implementation uses the `SomHunter::get_topKNN_display` from `core/src/SomHunter.cpp` runs through the dataset feature vectors (available in `features` member variable) and selects the ones closest to the `selected_image`.

## 2. Creating the N-API wrapper

There is no new functionality to wrap in this particular case, because the `get_display` function is already wrapped. If you implemented your own API functions, you would need to wrap them similarly as here.

The N-API wrappers `core/SomHunterNapi.h` and `.cpp` are extended with a N-API version of the function:

```cpp
Napi::Value get_display(const Napi::CallbackInfo &info);
```

The parameters are passed in `info`, these need to be converted to C++ types. The following code retrieves the first parameter as `std::string`:
```cpp
info[0].As<Napi::String>().Utf8Value()
```

From the N-Api wrapper, you can call the actual function in the backend instance (available as `somhunter`):
```cpp
FramePointerRange dislpay_frames =
  somhunter->get_display(disp_type, selected_image, page_num);
```

The output must be converted back to N-API values:
```cpp
napi_value result;
napi_create_object(env, &result); //convert `result` to a JS object

// Set some values in the object
{
  napi_value key; 
  napi_create_string_utf8(env, "page", NAPI_AUTO_LENGTH, &key);
  napi_value value;
  napi_create_uint32(env, uint32_t(page_num), &value);
  napi_set_property(env, result, key, value);
}

// ... fill in more values

return Napi::Object(env, result); //and return it
```

Consult [N-API documentation](https://nodejs.org/api/n-api.html) for more details on translating JavaScript objects to C++.

## 3. Adding the front-end UI functionality

The k-NN functionality uses a tiny button in each displayed frame on each display, which the user can click for switching to the actual k-NN display. The frame thumbnail can be modified in `views/somhunter_event_handlers.ejs`; we have added the button as such:

```js
function getThumbPrototype(likedStr, actionStr, id, src) {
  //...
    <a 
      class="button frame-in-grid-hover-btn show-knn" 
      onclick="showTopDisplay('topknn', ${id});event.cancelBubble=true;" 
      title="Show most similiar frames.">
      KNN
    </a>
  //...
}
```

The newly generated on-click event requires a handler, `showTopDisplay`, which is defined in the same file. This handler creates a request that is sent to the front-end and waits for the response, so that it can update the page accordingly:
```js
function showTopDisplay(type, id, thisFilename) {
  pageId = 0;
  if (type === undefined) type = "topn";
  let url = "/get_top_screen?pageId=" + pageId + "&type=" + type;
  if (id !== undefined) url += "&frameId=" + id;

  fetch(url, {
    method: "GET",
    headers: {
      "Content-Type": "application/json",
    },
  })
    .then((res) => res.json())
    .then((data) => {
      if (data.error) throw Error(data.error.message);
      
      viewData = data.viewData;
      putDocumentToState(viewData);
    })
    .catch((e) => {
      //...
    });
}
```

This sends the `/get_top_screen` request to the frontend whenever the user clicks the button. The frontend handles the request using the standard Node routing mechanism. The route is mapped to the query string in `app.js`:
```js
app.get("/get_top_screen", endpoints.getTopScreen);
```

The last missing part is the actual request handler that is called by Node router, and forwards the request to the back-end. That is defined in `routes/endpoints.js` as such:
```js
exports.getTopScreen = function (req, res) {
  const sess = req.session;

  global.logger.log("info", req.query)
  let type = 'topn'
  let pageId = 0;
  let frameId = 0;
  if (req.query) {
    if (req.query.type) type = req.query.type;
    if (req.query.pageId) pageId = Number(req.query.pageId);
    if (req.query.frameId) frameId = Number(req.query.frameId);
  }

  let frames = [];
 
  const displayFrames =
    global.core.getDisplay(global.cfg.framesPathPrefix, type, pageId, frameId);

  frames = displayFrames.frames;

  SessionState.switchScreenTo(sess.state, type, frames, frameId);

  let viewData = {};
  viewData.somhunter = SessionState.getSomhunterUiState(sess.state);

  res.status(200).jsonp({ viewData: viewData });
};
```
