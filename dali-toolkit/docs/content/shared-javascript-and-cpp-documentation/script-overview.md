<!--
/**-->

# Scripting Overview  {#scriptoverview}

DALi has:
- JSON to support:
 - layouting
 - theme / styling
 - templated actor/control creation
 - basic actions
 - DALi GUI builder generates JSON files. Allows UI designers to create / modify the application look and feel.

- JavaScript to support:
 - Rapid Application Development
 - Hybrid C++/JavaScript applications
 - Leaverage third party JavaScript modules (backbone.js etc)

JSON support is built in to DALi.

JavaScript support is via a plugin held in DALi Toolkit, which builds automatically if Google's V8 engine is installed. 
The V8 version required by DALi can be built and installed using dali-core/scripts/dali_env script.

Files can be loaded inside any DALi application, or from command line using the launcher ( part of dali-demo).

~~~{.cpp}
scripting.example hello-world.json hello-world.js
~~~

We currently have JSON and JavaScript example files held in dali-demo/resources/scripts
  
![ ](../assets/img/javascript-wrapping-guide/scripting-overview.png)
![ ](scripting-overview.png)

# JSON

JSON file contains different sections:
- **Templates** actor & control tree creation
- **Styles** used to style actor & control trees
- **Animations**
- **Instances** of objects for path, shader-effects, render-tasks, frame-buffers
- **Stage**. A list of actors / controls that can be added to the stage
- **Constants**  (e.g. positions / colors, that can be references by other parts of the JSON file);
- **Actions**
  
## Examples

### JSON templates section
  
~~~{.json}

    “templates”:
    {
      "name":"users",
      "type":"Actor",
      "parent-origin":"TOP_CENTER",
      "anchor-point":"TOP_CENTER",
      "size":"{DEFAULT_MENU_USER_SIZE}",
      "color-mode":"USE_OWN_COLOR",
      "actors":
      [
        {
          "name":"usersBackground",
          "type":"ImageActor",
          "parent-origin":"CENTER",
          "anchor-point":"CENTER",
          "size":"{DEFAULT_MENU_USER_SIZE}",
          "color-mode":"USE_OWN_COLOR",
          "image":{ "filename":"{IMAGE_PATH}white-pixel.png" },
          "color":"{DEFAULT_MENU_BACKGROUND_COLOR}"
        },
        {
          "name":"icon",
          "type":"ImageActor",
          "parent-origin":"TOP_CENTER",
          "anchor-point":"TOP_CENTER",
          "position":[0,41,1],
          "image":{ "filename":"{IMAGE_PATH}ico_man_nor.png" }
        },
      ]
    },


~~~

#### JavaScript example

~~~{.js}

var builder = new dali.Builder();

builder.loadFromFile( "my-app.json");

var userActorTree = builder.create( { template:"users"} );

~~~

#### C++ example

~~~{.cpp}
Builder builder = Builder::New();

std::string jsonData = loadFile("my-app.json");

builder.LoadFromString( jsonData );

Actor userActorTree = builder.Create("users");
~~~


### JSON styles section

~~~{.json}
“styles”:
  {
    “live-tv-focus":
    {
      "actors":
      {
        "background":
        {
          "image":{ "filename":"{IMAGE_PATH}live_tv_background.png" },
          "color":"{DEFAULT_MENU_ITEM_COLOR_1}"
        },
        "icon":
        {
          "image":{ "filename":"{IMAGE_PATH}icn_live_tv_foc.png" }
        },
        "label":
        {
          "color-alpha":1,
          "text":"<font size='20' weight='bold'><b>LIVE</b></font>"
        }
      }
    },
  }
~~~

#### JavaScript example

~~~{.js}
builder.applyStyle( "live-tv-focus", tvIcon );
~~~

#### C++ example

~~~{.cpp}
builder.ApplyStyle( "live-tv-focus", tvIcon );
~~~

### JSON animations section

~~~{.json}
"animations":
  {
    "animate-show":
    {
      "duration":0.5,
      "properties":
      [
        {
          "actor":"background",
          "property":"position-x",
          "value":30,
          "alpha-function":"EASE_IN_OUT"
        },
        {
          "actor":"itemsBackground",
          "property":"color-alpha",
          "value":0.85,
          "time-period": {"delay": 0.25, "duration": 0.25 },
          "alpha-function":"EASE_IN_OUT"
        },
        {
          "actor":"usersBackground",
          "property":"color-alpha",
          "value":0.85,
          "time-period": {"delay": 0.25, "duration": 0.25 },
          "alpha-function":"EASE_IN_OUT"
        }
      ]
    },
~~~

#### JavaScript example

~~~{.js}
// JavaScript

var anim = builder.createAnimation( { animation:"animate-show", actor: myActor } );
~~~

~~~{.cpp}
// C+++

Animation anim = builder.createAnimation( "animate-show", propertyMap );
~~~

### JSON stage section

~~~{.json}

“stage": [{
      "name":"simple-table",
      "type":"TableView",
      "background-color": [0.5,0.5,0,1],
      "parent-origin": "CENTER",
      "size":[400,500,1],
      "rows": 4,
      "columns":4,
      "cell-padding": [10, 5],
      "layout-animation-duration": 0.5,
      "layout-rows": {  // set the height of the rows
        "0": { "policy": "fixed", "value": 40 },
        "1": { "policy": "relative", "value": 0.33 },
        "2": { "policy": "fixed", "value": 120 }
      },
      "layout-columns": { // set the widths of the columns
        "1": { "policy": "fixed", "value": 150 },
        "2": { "policy": "relative", "value": 0.35 },
        "3": { "policy": "relative", "value": 0.15 }
      },
      "actors": [{
          "name":"gallery-1",
          "type":"ImageActor",
          "image": {
            "filename": "{DALI_IMAGE_DIR}gallery-large-1.jpg"
          },
          "custom-properties": { // properties registered dynamically
            "cell-indices": [0,0],// property to specify the top-left cell this child occupies
            "row-span":4, // property to specify how many rows this child occupies, if not set, default value is 1
            "column-spam":1 // property to specify how many columns this child occupies
            ....
~~~

#### JavaScript example

~~~{.js}
// add all actors under stage section to the root layer
builder.addActors( dali.stage.getRootLayer() );
~~~

#### C++ example

~~~{.cpp}
// add all actors under stage section to the root layer
builder.AddActors( Stage::GetCurrent().GetRootLayer() );
~~~

# JavaScript

For the JavaScript API please build dali-toolkit with YUIDOC installed. This will generate the documentation.
See dali-toolkit/plugins/dali-script-v8/docs/README.txt

To execute JavaScript from C++

~~~{.cpp}
script = Toolkit::Script::New();

script.ExecuteFile( scriptFileName );
~~~

@class _Guide_JSON_and_JavaScript_overview

*/