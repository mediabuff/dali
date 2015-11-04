/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <stdlib.h>
#include <dali-toolkit-test-suite-utils.h>
#include <dali-toolkit/devel-api/builder/builder.h>
#include <dali/integration-api/events/touch-event-integ.h>

#define STRINGIFY(A)#A

using namespace Dali;
using namespace Toolkit;

namespace
{

std::string ReplaceQuotes(const std::string &in_s)
{
  std::string s(in_s);
  // wrong as no embedded quote but had regex link problems
  std::replace(s.begin(), s.end(), '\'', '"');
  return s;
}

struct BuilderFunctor
{
  BuilderFunctor( bool& called ) : mCalled( called )
  {
    mCalled = false;
  }

  void operator()()
  {
    mCalled = true;
  }

  bool& mCalled;
};

} // namespace



void builder_startup(void)
{
  test_return_value = TET_UNDEF;
}

void builder_cleanup(void)
{
  test_return_value = TET_PASS;
}

int UtcDaliBuilderQuitSignal(void)
{
  ToolkitTestApplication application;

  // JSON with a quit event when the actor is touched
  std::string json(
      "{"
         "\"stage\":"
         "[{"
           "\"type\": \"Actor\","
           "\"size\": [100,100,1],"
           "\"parent-origin\": \"TOP_LEFT\","
           "\"anchor-point\": \"TOP_LEFT\","
           "\"signals\": [{"
             "\"name\": \"touched\","
             "\"action\": \"quit\""
           "}]"
         "}]"
      "}"
  );
  Builder builder = Builder::New();
  builder.LoadFromString( json );
  builder.AddActors ( Stage::GetCurrent().GetRootLayer() );

  // Connect to builder's quit signal
  bool functorCalled( false );
  builder.QuitSignal().Connect( &application, BuilderFunctor( functorCalled ) );

  // Render and notify
  application.SendNotification();
  application.Render();

  // Emit touch event and check that our quit method is called
  Integration::TouchEvent touchEvent;
  touchEvent.points.push_back( TouchPoint ( 0, TouchPoint::Down, 10.0f, 10.0f ) );
  application.ProcessEvent( touchEvent );
  DALI_TEST_CHECK( functorCalled );

  END_TEST;
}


int UtcDaliBuilderAnimationP(void)
{
  ToolkitTestApplication application;

  // JSON with a quit event when the actor is touched
  std::string json(
        "{"
        "   \"constants\":"
        "   {"
        "     \"ALPHA_FUNCTION\":\"EASE_IN_OUT\""
        "   },"
        "   \"paths\":"
        "   {"
        "     \"path0\":"
        "     {"
        "       \"points\":[ [-150, -50, 0], [0.0,70.0,0.0], [190.0,-150.0,0.0] ],"
        "       \"curvature\":0.35"
        "     }"
        "   },"
        "  \"animations\": {"
        "    \"animate\": {"
        "      \"loop\": true,"
        "      \"end-action\": \"BAKE\","
        "      \"disconnect-action\": \"BAKE\","
        "      \"properties\":"
        "      [{"
        "        \"actor\": \"greeting\","
        "        \"property\": \"position\","
        "        \"value\": [300, 300, -1000],"
        "        \"alpha-function\": \"{ALPHA_FUNCTION}\","
        "        \"relative\": true,"
        "        \"time-period\": {"
        "          \"delay\": 0,"
        "          \"duration\": 3"
        "        }"
        "      },"
        "       {"
        "         \"actor\": \"greeting\","
        "         \"property\": \"visible\","
        "         \"alpha-function\": \"LINEAR\","
        "         \"value\": true"
        "       },"
        "       {"
        "         \"actor\": \"greeting\","
        "         \"property\": \"size-width\","
        "         \"alpha-function\": \"REVERSE\","
        "         \"value\": 10.0"
        "       },"
        "       {"
        "         \"actor\": \"greeting\","
        "         \"property\": \"orientation\","
        "         \"alpha-function\": \"EASE_IN\","
        "         \"value\": [10.0,20.0,30.0]"
        "       },"
        "       {"
        "         \"actor\": \"greeting\","
        "         \"property\": \"orientation\","
        "         \"alpha-function\": \"EASE_OUT\","
        "         \"value\": [0.0, 0.0, 0.0, 1.0]"
        "       },"
        "       {"
        "         \"actor\": \"greeting\","
        "         \"property\": \"orientation\","
        "         \"alpha-function\": \"EASE_IN_OUT\","
        "         \"value\": [0.0, 0.0, 0.0, 1.0]"
        "       },"
        "       {"
        "         \"actor\": \"greeting\","
        "         \"property\": \"orientation\","
        "         \"alpha-function\": \"EASE_IN_SINE\","
        "         \"value\": [0.0, 0.0, 0.0, 1.0]"
        "       },"
        "       {"
        "         \"actor\": \"greeting\","
        "         \"property\": \"orientation\","
        "         \"alpha-function\": \"EASE_OUT_SINE\","
        "         \"value\": [0.0, 0.0, 0.0, 1.0]"
        "       },"
        "       {"
        "         \"actor\": \"greeting\","
        "         \"property\": \"orientation\","
        "         \"alpha-function\": \"EASE_IN_OUT_SINE\","
        "         \"value\": [0.0, 0.0, 0.0, 1.0]"
        "       },"
        "       {"
        "         \"actor\": \"greeting\","
        "         \"property\": \"orientation\","
        "         \"alpha-function\": \"BOUNCE\","
        "         \"value\": [0.0, 0.0, 0.0, 1.0]"
        "       },"
        "       {"
        "         \"actor\": \"greeting\","
        "         \"property\": \"orientation\","
        "         \"alpha-function\": \"SIN\","
        "         \"value\": [0.0, 0.0, 0.0, 1.0]"
        "       },"
        "       {"
        "         \"actor\": \"greeting\","
        "         \"property\": \"orientation\","
        "         \"alpha-function\": \"EASE_OUT_BACK\","
        "         \"value\": [0.0, 0.0, 0.0, 1.0]"
        "       }"
        "      ]"
        "    },"
        "    \"path-animation\": {"
        "      \"duration\": 3.0,"
        "      \"end-action\": \"DISCARD\","
        "      \"disconnect-action\": \"BAKE_FINAL\","
        "      \"properties\": [{"
        "        \"actor\": \"greeting\","
        "        \"path\":\"path0\","
        "        \"forward\":[1,0,0],"
        "        \"alpha-function\": \"EASE_IN_OUT\","
        "        \"time-period\": {"
        "          \"delay\": 0,"
        "          \"duration\": 3"
        "        }"
        "      }]"
        "    }"
        "  },"
        "  \"stage\": [{"
        "    \"name\": \"greeting\","
        "    \"type\": \"TextLabel\","
        "    \"text\": \"Touch me\","
        "    \"styles\": [\"basic-text\"],"
        "    \"position\": [0, -120, 0],"
        "    \"size\": [200, 200, 1],"
        "    \"orientation\": [0, 0, 30],"
        "    \"signals\": [{"
        "      \"name\": \"touched\","
        "      \"action\": \"play\","
        "      \"animation\": \"animate\""
        "    }]"
        "  }]"
        "}");

  Builder builder = Builder::New();
  builder.LoadFromString( json );
  builder.AddActors( Stage::GetCurrent().GetRootLayer() );

  Animation anim = builder.CreateAnimation("animate");

  DALI_TEST_CHECK( anim );

  Property::Map map;
  map["ALPHA_FUNCTION"] = "EASE_IN_SQUARE";
  anim = builder.CreateAnimation("animate", map);

  DALI_TEST_CHECK( anim );

  anim = builder.CreateAnimation("path-animation");

  DALI_TEST_CHECK( anim );

  // trigger play
  // Emit touch event and check that our quit method is called
  Integration::TouchEvent touchEvent;
  touchEvent.points.push_back( TouchPoint ( 0, TouchPoint::Down, 10.0f, 10.0f ) );
  application.ProcessEvent( touchEvent );

  // Render and notify
  application.SendNotification();
  application.Render();


  END_TEST;
}

int UtcDaliBuilderAnimationN(void)
{
  ToolkitTestApplication application;

  // JSON with a quit event when the actor is touched
  std::string json(
        "{"
        "   \"constants\":"
        "   {"
        "     \"TEXT\": \"Touch Me\","
        "     \"NAME\": \"greeting\" "
        "   },"
        "   \"paths\":"
        "   {"
        "     \"path0\":"
        "     {"
        "       \"points\":[ [-150, -50, 0], [0.0,70.0,0.0], [190.0,-150.0,0.0] ],"
        "       \"curvature\":0.35"
        "     }"
        "   },"
        "  \"animations\": {"
        "    \"animate\": {"
        "      \"loop\": true,"
        "      \"end-action\": \"BAKE\","
        "      \"disconnect-action\": \"BAKE\","
        "      \"properties\":"
        "      [{"
        "        \"actor\": \"{NAME}\","
        "        \"property\": \"positioninvalid\","
        "        \"value\": [300, 300, -1000],"
        "        \"alpha-function\": \"EASE_IN_OUT\","
        "        \"relative\": true,"
        "        \"time-period\": {"
        "          \"delay\": 0,"
        "          \"duration\": 3"
        "        }"
        "      }"
        "      ]"
        "    },"
        "    \"animate2\": {"
        "      \"loop\": true,"
        "      \"end-action\": \"BAKE\","
        "      \"disconnect-action\": \"BAKE\","
        "      \"properties\":"
        "      [{"
        "        \"actor\": \"{NAME}\","
        "        \"property\": \"positioninvalid\","
        "        \"value\": [300, 300, -1000],"
        "        \"alpha-function\": \"EGGS_OVER_EASY\","
        "        \"relative\": true,"
        "        \"time-period\": {"
        "          \"delay\": 0,"
        "          \"duration\": 3"
        "        }"
        "      }"
        "      ]"
        "    },"
        "    \"path-animation\": {"
        "      \"duration\": 3.0,"
        "      \"end-action\": \"DISCARD\","
        "      \"disconnect-action\": \"BAKE_FINAL\","
        "      \"properties\": [{"
        "        \"actor\": \"greeting\","
        "        \"path\":\"pathDoesntExist\","
        "        \"forward\":[1,0,0],"
        "        \"alpha-function\": \"EASE_IN_OUT\","
        "        \"time-period\": {"
        "          \"delay\": 0,"
        "          \"duration\": 3"
        "        }"
        "      }]"
        "    }"
        "  },"
        "  \"stage\": [{"
        "    \"name\": \"greeting\","
        "    \"type\": \"TextLabel\","
        "    \"text\": \"Touch me\","
        "    \"styles\": [\"basic-text\"],"
        "    \"position\": [0, -120, 0],"
        "    \"size\": [200, 200, 1],"
        "    \"orientation\": [0, 0, 30],"
        "    \"signals\": [{"
        "      \"name\": \"touched\","
        "      \"action\": \"play\","
        "      \"animation\": \"animate\""
        "    }]"
        "  },"
        "  {"
        "    \"name\": \"greeting2\","
        "    \"type\": \"TextLabel\","
        "    \"text\": \"Touch me\""
        "  }]"
        "}");


  Builder builder = Builder::New();
  builder.LoadFromString( json );
  builder.AddActors( Stage::GetCurrent().GetRootLayer() );

  Animation anim = builder.CreateAnimation("animate");

  // log warning line coverage
  anim = builder.CreateAnimation("path-animation");
  DALI_TEST_CHECK(anim);

  anim = builder.CreateAnimation("animate");
  DALI_TEST_CHECK(anim);

  anim = builder.CreateAnimation("animate2");
  DALI_TEST_CHECK(anim);

  // create referencing a different actor aka animation templates
  Property::Map map;
  map["NAME"] = "greeting2";
  anim = builder.CreateAnimation("animate2", map);
  DALI_TEST_CHECK(anim);

  // alternative actor to use for FindChildByName
  anim = builder.CreateAnimation("animate2", Dali::Stage::GetCurrent().GetRootLayer());
  DALI_TEST_CHECK(anim);

  // alternative actor to use for FindChildByName
  anim = builder.CreateAnimation("animate2", map, Dali::Stage::GetCurrent().GetRootLayer());
  DALI_TEST_CHECK(anim);


  END_TEST;

}

int UtcDaliBuilderConstantsP(void)
{
  ToolkitTestApplication application;

  // JSON with a quit event when the actor is touched
  std::string json(
      "{"
      "\"constants\":"
      "{"
      "  \"IMAGE_PATH\": \"apath\","
      "  \"WIDTH\": 22.3,"
      "  \"ANCHOR\": \"TOP_LEFT\","
      "  \"PADDING\": [1,2,3,4]"
      "},"
      "\"stage\":"
      "[{"
      "  \"type\": \"ImageActor\","
      "  \"name\": \"{NAME}\","
      "  \"size\": [100,100,1],"
      "  \"parent-origin\": \"TOP_LEFT\","
      "  \"anchor-point\": \"{ANCHOR}\","
      "  \"padding\": \"{PADDING}\","
      "  \"image\": { \"filename\": \"dir/{IMAGE_PATH}\" },"
      "  \"size-width\": \"{WIDTH}\","
      "  \"signals\": [{"
      "    \"name\": \"touched\","
      "    \"action\": \"quit\""
      "  }]"
      "}]"
      "}"
  );

  Builder builder = Builder::New();
  builder.LoadFromString( json );

  builder.AddConstant( "NAME", "image" );

  Property::Map map = builder.GetConstants();

  Dali::Property::Value* pValue = map.Find( "NAME" );

  DALI_TEST_CHECK( pValue );

  pValue = map.Find( "IMAGE_PATH" );

  DALI_TEST_CHECK( pValue );

  Dali::Property::Value value = builder.GetConstant( "WIDTH" );

  DALI_TEST_CHECK( value.GetType() != Property::NONE );

  builder.AddActors ( Stage::GetCurrent().GetRootLayer() );
  DALI_TEST_CHECK( builder );

  Actor actor = Stage::GetCurrent().GetRootLayer().FindChildByName("image");
  DALI_TEST_CHECK( actor );

  END_TEST;
}

int UtcDaliBuilderTemplatesAndStylesP(void)
{
  ToolkitTestApplication application;

  // JSON with a quit event when the actor is touched
  std::string json(
      "{\n"
      "\"constants\":"
      "{"
      "  \"SIZE\": [10,20,30]"
      "},"
      "\"styles\":\n"
      "{\n"
      "  \"image-style\": \n"
      "  {\n"
      "    \"color\": [1,0,0,1],\n"
      "    \"actors\": {\n"
      "      \"child-image\": {\n"
      "        \"color\": [0,1,0,1]\n"
      "      }\n"
      "    }\n"
      "  }\n"
      "},\n"
      "\"templates\":\n"
      "{\n"
      "  \"image-tree\": { \n"
      "    \"type\": \"ImageActor\",\n"
      "    \"styles\": [\"image-style\"],\n"
      "    \"name\": \"image\",\n"
      "    \"size\": \"{SIZE}\",\n"
      "    \"signals\": [{\n"
      "      \"name\": \"touched\",\n"
      "      \"action\": \"quit\"\n"
      "    }],\n"
      "    \"actors\": [\n"
      "      {\n"
      "        \"type\":\"ImageActor\",\n"
      "        \"name\":\"child-image\" \n"
      "      }\n"
      "    ]\n"
      "  }\n"
      "},\n"
      "\"stage\":"
      "[{"
      "  \"type\": \"image-tree\","
      "  \"size\": [100,100,1]"
      "}]"
      "}\n"
  );

  std::string stylejson(
    "{\n"
    " \"color\": [1,0,0,1],\n"
    " \"actors\": {\n"
    "   \"child-image\": {\n"
    "     \"color\": [0,1,0,1]\n"
    "   }\n"
    " }\n"
    "}\n"
    );

  std::string templatejson(
    "{ \n"
    "  \"type\": \"ImageActor\",\n"
    "  \"styles\": [\"image-style\"],\n"
    "  \"name\": \"image\",\n"
    "  \"size\": \"{SIZE}\",\n"
    "  \"signals\": [{\n"
    "    \"name\": \"touched\",\n"
    "    \"action\": \"quit\"\n"
    "  }],\n"
    "  \"actors\": [\n"
    "    {\n"
    "      \"type\":\"ImageActor\",\n"
    "      \"name\":\"child-image\" \n"
    "    }\n"
    "  ]\n"
    "}\n"
    );

  Builder builder = Builder::New();
  builder.LoadFromString( json );

  ImageActor actor = ImageActor::DownCast( builder.Create( "image-tree" ) );
  DALI_TEST_CHECK( actor );

  Dali::Property::Map map;
  map["SIZE"] = Vector3(100,100,1);
  actor = ImageActor::DownCast( builder.Create( "image-tree", map ) );
  DALI_TEST_CHECK( actor );

  // create from json snippet
  actor = ImageActor::DownCast( builder.CreateFromJson( templatejson ) );
  DALI_TEST_CHECK( actor );


  // NB: already applied in create
  DALI_TEST_CHECK( builder.ApplyStyle( "image-style", actor ) );

  // apply from json snippet
  DALI_TEST_CHECK( builder.ApplyFromJson( actor, stylejson ) );

  END_TEST;
}

int UtcDaliBuilderRenderTasksP(void)
{
  ToolkitTestApplication application;

  // JSON with a quit event when the actor is touched
  std::string json(
      "{\n"
      "\"render-tasks\":\n"
      "{\n"
      "  \"task0\": {\n"
      "    \"source-actor\": \"image\",\n"
      "    \"camera-actor\": \"camera\" \n"
      "  }\n"
      "},\n"
      "\"stage\":\n"
      "[\n"
      "  { \n"
      "    \"type\": \"CameraActor\",\n"
      "    \"name\": \"image\"\n"
      "  }, \n"
      "  { \n"
      "    \"type\": \"ImageActor\",\n"
      "    \"name\": \"image\",\n"
      "    \"size\": [100,100,1],\n"
      "    \"signals\": [{\n"
      "      \"name\": \"touched\",\n"
      "      \"action\": \"quit\"\n"
      "    }],\n"
      "    \"actors\": [\n"
      "      {\n"
      "        \"type\":\"ImageActor\",\n"
      "        \"name\":\"child-image\" \n"
      "      }\n"
      "    ]\n"
      "  }\n"
      "]\n"
      "}\n"
  );

  Builder builder = Builder::New();
  builder.LoadFromString( json );

  unsigned int count = Stage::GetCurrent().GetRenderTaskList().GetTaskCount();

  // coverage
  builder.CreateRenderTask( "task0" );

  DALI_TEST_CHECK( count <
                   Stage::GetCurrent().GetRenderTaskList().GetTaskCount() );

  END_TEST;
}

int UtcDaliBuilderChildActionP(void)
{
  ToolkitTestApplication application;

  // JSON with a quit event when the actor is touched
  std::string json(
      "{\n"
      "  \"stage\":\n"
      "  [{\n"
      "    \"type\": \"Actor\",\n"
      "    \"name\": \"actor\",\n"
      "    \"size\": [100,100,1],\n"
      "    \"parent-origin\": \"TOP_LEFT\",\n"
      "    \"anchor-point\": \"TOP_LEFT\",\n"
      "    \"actors\": [{\n"
      "      \"type\": \"Actor\",\n"
      "      \"name\": \"sub-actor\"\n"
      "    }],\n"
      "    \"signals\": [{\n"
      "      \"name\": \"touched\",\n"
      "      \"action\": \"hide\",\n"
      "      \"actor\": \"actor\",\n"
      "      \"child-actor\": \"sub-actor\"\n"
      "    }]\n"
      "  }]\n"
      "}\n"
  );

  Builder builder = Builder::New();
  builder.LoadFromString( json );
  builder.AddActors ( Stage::GetCurrent().GetRootLayer() );

  // Render and notify
  application.SendNotification();
  application.Render();

  // Emit touch event and check that our quit method is called
  Integration::TouchEvent touchEvent;
  touchEvent.points.push_back( TouchPoint ( 0, TouchPoint::Down, 10.0f, 10.0f ) );
  application.ProcessEvent( touchEvent );

  // Render and notify
  application.SendNotification();
  application.Render();

  Actor actor = Stage::GetCurrent().GetRootLayer().FindChildByName("sub-actor");
  DALI_TEST_CHECK( actor );

  DALI_TEST_CHECK( !actor.IsVisible() );

  END_TEST;
}

int UtcDaliBuilderSetPropertyActionP(void)
{
  ToolkitTestApplication application;

  // JSON with a quit event when the actor is touched
  std::string json(
      "{\n"
      "  \"stage\":\n"
      "  [{\n"
      "    \"type\": \"Actor\",\n"
      "    \"name\": \"actor\",\n"
      "    \"size\": [100,100,1],\n"
      "    \"parent-origin\": \"TOP_LEFT\",\n"
      "    \"anchor-point\": \"TOP_LEFT\",\n"
      "    \"actors\": [{\n"
      "      \"type\": \"Actor\",\n"
      "      \"name\": \"sub-actor\"\n"
      "    }],\n"
      "    \"signals\": [{\n"
      "      \"name\": \"touched\",\n"
      "      \"action\": \"set\",\n"
      "      \"actor\": \"sub-actor\",\n"
      "      \"property\": \"visible\",\n"
      "      \"value\": false\n"
      "    }]\n"
      "  }]\n"
      "}\n"
  );

  Builder builder = Builder::New();
  builder.LoadFromString( json );
  builder.AddActors ( Stage::GetCurrent().GetRootLayer() );

  // Render and notify
  application.SendNotification();
  application.Render();

  // Emit touch event and check that our quit method is called
  Integration::TouchEvent touchEvent;
  touchEvent.points.push_back( TouchPoint ( 0, TouchPoint::Down, 10.0f, 10.0f ) );
  application.ProcessEvent( touchEvent );

  // Render and notify
  application.SendNotification();
  application.Render();

  Actor actor = Stage::GetCurrent().GetRootLayer().FindChildByName("sub-actor");
  DALI_TEST_CHECK( actor );

  DALI_TEST_CHECK( !actor.IsVisible() );

  END_TEST;
}

int UtcDaliBuilderGenericActionP(void)
{
  ToolkitTestApplication application;

  // JSON with a quit event when the actor is touched
  std::string json(
      "{\n"
      "  \"stage\":\n"
      "  [{\n"
      "    \"type\": \"Actor\",\n"
      "    \"name\": \"actor\",\n"
      "    \"size\": [100,100,1],\n"
      "    \"parent-origin\": \"TOP_LEFT\",\n"
      "    \"anchor-point\": \"TOP_LEFT\",\n"
      "    \"actors\": [{\n"
      "      \"type\": \"Actor\",\n"
      "      \"name\": \"sub-actor\"\n"
      "    }],\n"
      "    \"signals\": [{\n"
      "      \"name\": \"touched\",\n"
      "      \"action\": \"hide\"\n"
      "    }]\n"
      "  }]\n"
      "}\n"
  );

  Builder builder = Builder::New();
  builder.LoadFromString( json );
  builder.AddActors ( Stage::GetCurrent().GetRootLayer() );

  // Render and notify
  application.SendNotification();
  application.Render();

  // Emit touch event and check that our quit method is called
  Integration::TouchEvent touchEvent;
  touchEvent.points.push_back( TouchPoint ( 0, TouchPoint::Down, 10.0f, 10.0f ) );
  application.ProcessEvent( touchEvent );

  // Render and notify
  application.SendNotification();
  application.Render();

  Actor actor = Stage::GetCurrent().GetRootLayer().FindChildByName("actor");
  DALI_TEST_CHECK( actor );

  DALI_TEST_CHECK( !actor.IsVisible() );

  END_TEST;
}

int UtcDaliBuilderPropertyNotificationP(void)
{
  ToolkitTestApplication application;

  // JSON with a quit event when the actor is touched
  std::string json(
      "{\n"
      "  \"stage\":\n"
      "  [{\n"
      "    \"type\": \"Actor\",\n"
      "    \"name\": \"actor\",\n"
      "    \"size\": [100,100,1],\n"
      "    \"parent-origin\": \"TOP_LEFT\",\n"
      "    \"anchor-point\": \"TOP_LEFT\",\n"
      "    \"actors\": [{\n"
      "      \"type\": \"Actor\",\n"
      "      \"name\": \"sub-actor\"\n"
      "    }],\n"
      "    \"signals\": [{\n"
      "      \"name\": \"touched\",\n"
      "      \"action\": \"hide\"\n"
      "    }],\n"
      "    \"notifications\": [{\n"
      "      \"property\": \"visible\",\n"
      "      \"condition\": \"False\",\n"
      "      \"action\": \"show\"\n"
      "    }]\n"
      "  }]\n"
      "}\n"
  );

  Builder builder = Builder::New();
  builder.LoadFromString( json );
  builder.AddActors ( Stage::GetCurrent().GetRootLayer() );

  // Render and notify
  application.SendNotification();
  application.Render();

  // Emit touch event and check that our quit method is called
  Integration::TouchEvent touchEvent;
  touchEvent.points.push_back( TouchPoint ( 0, TouchPoint::Down, 10.0f, 10.0f ) );
  application.ProcessEvent( touchEvent );

  // Render and notify
  application.SendNotification();
  application.Render();

  // Render and notify
  application.SendNotification();
  application.Render();

  Actor actor = Stage::GetCurrent().GetRootLayer().FindChildByName("actor");
  DALI_TEST_CHECK( actor );

  DALI_TEST_CHECK( actor.IsVisible() );

  END_TEST;
}

int UtcDaliBuilderCustomPropertyP(void)
{
  ToolkitTestApplication application;

  // JSON with a quit event when the actor is touched
  std::string json(
      "{\n"
      "\"templates\":\n"
      "{\n"
      "  \"image-tree\": { \n"
      "    \"type\": \"ImageActor\",\n"
      "    \"name\": \"image\",\n"
      "    \"size\": [100,100,1],\n"
      "    \"signals\": [{\n"
      "      \"name\": \"touched\",\n"
      "      \"action\": \"quit\"\n"
      "    }],\n"
      "    \"custom-properties\": {\n"
      "      \"newproperty\": true\n"
      "    },\n"
      "    \"actors\": [\n"
      "      {\n"
      "        \"type\":\"ImageActor\",\n"
      "        \"name\":\"child-image\" \n"
      "      }\n"
      "    ]\n"
      "  }\n"
      "}\n"
      "}\n"
  );

  Builder builder = Builder::New();
  builder.LoadFromString( json );

  ImageActor actor = ImageActor::DownCast( builder.Create( "image-tree" ) );
  DALI_TEST_CHECK( actor );

  // NB: already applied in create
  Property::Index index = actor.GetPropertyIndex("newproperty");
  DALI_TEST_CHECK( Property::INVALID_INDEX != index );
  Property::Value value = actor.GetProperty(index);
  DALI_TEST_CHECK( value.Get<bool>() == true );

  END_TEST;
}

int UtcDaliBuilderShaderEffectP(void)
{
  ToolkitTestApplication application;

  // JSON with a quit event when the actor is touched
  std::string json(
    "{\n"
    "  \"stage\": [\n"
    "    {\n"
    "      \"type\": \"ImageActor\",\n"
    "      \"name\": \"Image1\",\n"
    "      \"position\": [\n"
    "        0.40461349487305,\n"
    "        0.9150390625,\n"
    "        0.0\n"
    "      ],\n"
    "      \"parent-origin\": [0.5, 0.5, 0.5],\n"
    "      \"size\": [200, 200, 0],\n"
    "      \"effect\": \"Ripple2D\",\n"
    "      \"image\": {\n"
    "        \"filename\": \"{DALI_IMAGE_DIR}gallery-medium-25.jpg\",\n"
    "        \"width\": 200,\n"
    "        \"height\": 80,\n"
    "        \"load-policy\": \"IMMEDIATE\",\n"
    "        \"release-policy\": \"NEVER\"\n"
    "      },\n"
    "      \"signals\": [\n"
    "        {\n"
    "          \"name\": \"on-stage\",\n"
    "          \"action\": \"play\",\n"
    "          \"animation\": \"Animation_1\"\n"
    "        }\n"
    "      ]\n"
    "    }\n"
    "  ],\n"
    "  \"paths\": {},\n"
    "  \"animations\": {\n"
    "    \"Animation_1\": {\n"
    "      \"loop\":true,\n"
    "      \"properties\": [\n"
    "        {\n"
    "          \"actor\": \"Image1\",\n"
    "          \"property\": \"uTime\",\n"
    "          \"value\": 10.0,\n"
    "          \"alpha-function\": \"LINEAR\",\n"
    "          \"time-period\": {\n"
    "            \"delay\": 0,\n"
    "            \"duration\": 10.0\n"
    "          },\n"
    "          \"gui-builder-timeline-color\": \"#8dc0da\"\n"
    "        }\n"
    "      ]\n"
    "    }\n"
    "  },\n"
    "  \"shader-effects\": {\n"
    "    \"Ripple2D\": {\n"
    "      \"program\": {\n"
    "        \"vertexPrefix\": \"\",\n"
    "        \"vertex\": \"void main(void)\\n{\\n  gl_Position = uProjection * uModelView * vec4(aPosition, 1.0);\\n  vTexCoord = aTexCoord;\\n}\\n\\n\",\n"
    "        \"fragmentPrefix\": \"\",\n"
    "        \"fragment\": \"precision mediump float;\\nuniform float uAmplitude; // 0.02; (< 1)\\nuniform float uTime;\\nvoid main()\\n{\\n  highp vec2 textureSize = sTextureRect.zw - sTextureRect.xy;\\n  highp vec2 pos = -1.0 + 2.0 * vTexCoord.st/textureSize;\\n  highp float len = length(pos);\\n  highp vec2 texCoord = vTexCoord.st/textureSize + pos/len * sin( len * 12.0 - uTime * 4.0 ) * uAmplitude; \\n  gl_FragColor = texture2D(sTexture, texCoord) * uColor;\\n}\\n\\n\\n\",\n"
    "        \"geometry-type\": \"GEOMETRY_TYPE_IMAGE\"\n"
    "      },\n"
    "      \"geometry-hints\": \"HINT_NONE\",\n"
    "      \"grid-density\": 0,\n"
    "      \"loop\": true,\n"
    "      \"uAmplitude\": 0.02,\n"
    "      \"uTime\": 0.0\n"
    "    }\n"
    "  }\n"
    "}\n"

  );

  Builder builder = Builder::New();
  builder.LoadFromString( json );

  ShaderEffect effect = builder.GetShaderEffect("Ripple2D");

  // coverage
  DALI_TEST_CHECK( effect );

  END_TEST;
}


int UtcDaliBuilderLoadFromStringN(void)
{
  ToolkitTestApplication application;

  // JSON with a quit event when the actor is touched
  std::string json(
      "asdfsadf dsf asdf asdf {"
         "\"stage\":"
         "[{"
           "\"type\": \"Actor\","
           "\"size\": [100,100,1],"
           "\"parent-origin\": \"TOP_LEFT\","
           "\"anchor-point\": \"TOP_LEFT\","
           "\"signals\": [{"
             "\"name\": \"touched\","
             "\"action\": \"quit\""
           "}]"
         "}]"
      "}"
  );
  Builder builder = Builder::New();

  bool assert1 = false;

  try
  {
    builder.LoadFromString( json );
  }
  catch( Dali::DaliException& e )
  {
    DALI_TEST_PRINT_ASSERT( e );
    DALI_TEST_EQUALS(e.condition, "!\"Cannot parse JSON\"", TEST_LOCATION);
    assert1 = true;
  }

  DALI_TEST_CHECK( assert1 );

  END_TEST;
}

int UtcDaliBuilderShaderEffect2P(void)
{
  ToolkitTestApplication application;

  // JSON with a quit event when the actor is touched
  std::string json(
    "{\n"
    "\"templates\":\n"
    "{\n"
    "  \"image-tree\": { \n"
    "    \"type\": \"ImageActor\",\n"
    "    \"size\": [100,100,1],\n"
    "    \"parent-origin\": [0.5, 0.5, 0.5],\n"
    "    \"position\": [\n"
    "      0.40461349487305,\n"
    "      0.9150390625,\n"
    "      0.0\n"
    "    ],\n"
    "    \"signals\": [{\n"
    "      \"name\": \"touched\",\n"
    "      \"action\": \"quit\"\n"
    "    }],\n"
    "    \"actors\": [\n"
    "      {\n"
    "        \"type\":\"ImageActor\",\n"
    "        \"name\":\"child-image\" \n"
    "      }\n"
    "    ]\n"
    "  }\n"
    "},\n"
    "  \"stage\": [\n"
    "    {\n"
    "      \"type\": \"image-tree\",\n"
    "      \"name\": \"Image1\",\n"
    "      \"effect\": \"Ripple2D\",\n"
    "      \"image\": \"offscreen\""
    "    }\n"
    "  ],\n"
    "  \"shader-effects\": {\n"
    "    \"Ripple2D\": {\n"
    "      \"program\": {\n"
    "        \"vertexPrefix\": \"\",\n"
    "        \"vertex\": \"void main(void)\\n{\\n  gl_Position = uProjection * uModelView * vec4(aPosition, 1.0);\\n  vTexCoord = aTexCoord;\\n}\\n\\n\",\n"
    "        \"fragmentPrefix\": \"\",\n"
    "        \"fragment\": \"precision mediump float;\\nuniform float uAmplitude; // 0.02; (< 1)\\nuniform float uTime;\\nvoid main()\\n{\\n  highp vec2 textureSize = sTextureRect.zw - sTextureRect.xy;\\n  highp vec2 pos = -1.0 + 2.0 * vTexCoord.st/textureSize;\\n  highp float len = length(pos);\\n  highp vec2 texCoord = vTexCoord.st/textureSize + pos/len * sin( len * 12.0 - uTime * 4.0 ) * uAmplitude; \\n  gl_FragColor = texture2D(sTexture, texCoord) * uColor;\\n}\\n\\n\\n\",\n"
    "        \"geometry-type\": \"GEOMETRY_TYPE_IMAGE\"\n"
    "      },\n"
    "      \"geometry-hints\": \"HINT_NONE\",\n"
    "      \"grid-density\": 0,\n"
    "      \"loop\": true,\n"
    "      \"uAmplitude\": 0.02,\n"
    "      \"uTime\": 0.0\n"
    "    }\n"
    "  },\n"
    "  \"frame-buffer-images\": {\n"
    "    \"offscreen\": {\n"
    "      \"type\": \"FrameBufferImage\","
    "      \"pixel-format\":\"RGBA8888\","
    "      \"width\": 400,"
    "      \"height\": 400"
    "    }"
    "   }"
    "}\n"

  );

  Builder builder = Builder::New();
  builder.LoadFromString( json );

  // coverage
  DALI_TEST_CHECK( true );

  END_TEST;
}

int UtcDaliBuilderAddActorsP(void)
{
  ToolkitTestApplication application;

  // JSON with a quit event when the actor is touched
  std::string json(
      "{\n"
      "  \"arbitarysection\":\n"
      "  [{\n"
      "    \"type\": \"Actor\",\n"
      "    \"name\": \"actor\",\n"
      "    \"size\": [100,100,1],\n"
      "    \"parent-origin\": \"TOP_LEFT\",\n"
      "    \"anchor-point\": \"TOP_LEFT\",\n"
      "    \"actors\": [{\n"
      "      \"type\": \"Actor\",\n"
      "      \"name\": \"sub-actor\",\n"
      "      \"visible\": false\n"
      "    }],\n"
      "    \"signals\": [{\n"
      "      \"name\": \"touched\",\n"
      "      \"action\": \"hide\",\n"
      "      \"actor\": \"actor\",\n"
      "      \"child-actor\": \"sub-actor\"\n"
      "    }]\n"
      "  }]\n"
      "}\n"
  );

  Builder builder = Builder::New();
  builder.LoadFromString( json );
  builder.AddActors ( "arbitarysection", Stage::GetCurrent().GetRootLayer() );

  // Render and notify
  application.SendNotification();
  application.Render();

  Actor actor = Stage::GetCurrent().GetRootLayer().FindChildByName("sub-actor");
  DALI_TEST_CHECK( actor );

  DALI_TEST_CHECK( !actor.IsVisible() );

  END_TEST;
}

int UtcDaliBuilderFrameBufferP(void)
{
  ToolkitTestApplication application;

  // JSON with a quit event when the actor is touched
  std::string json(
    "{\n"
    "  \"constants\":\n"
    "  {\n"
    "    \"FB_WIDTH\": 200.0,\n"
    "    \"FB_HEIGHT\": 200.0,\n"
    "    \"FB_SIZE\": [200,200],\n"
    "    \"FB_ASPECT_RATIO\": 1\n"
    "  },\n"
    "  \"stage\": [\n"
    "    {\n"
    "      \"type\": \"ImageActor\",\n"
    "      \"name\": \"fbOnStage\",\n"
    "      \"position\": [\n"
    "        0.40461349487305,\n"
    "        0.9150390625,\n"
    "        0.0\n"
    "      ],\n"
    "      \"parent-origin\": [0.5, 0.5, 0.5],\n"
    "      \"size\": [300, 300, 0],\n"
    "      \"image\": \"fb0\",\n"
    "      \"clear-color\": [1,0,0,1]\n"
    "    },\n"
    "    {\n"
    "      \"type\": \"ImageActor\",\n"
    "      \"name\": \"Image1\",\n"
    "      \"size\": [200, 200, 0],\n"
    "      \"parent-origin\": [0.5, 0.5, 0.5],\n"
    "      \"effect\": \"Ripple2D\",\n"
    "      \"image\": {\n"
    "        \"filename\": \"{DALI_IMAGE_DIR}gallery-medium-25.jpg\",\n"
    "        \"width\": 200,\n"
    "        \"height\": 80,\n"
    "        \"load-policy\": \"IMMEDIATE\",\n"
    "        \"release-policy\": \"NEVER\"\n"
    "      },\n"
    "      \"signals\": [\n"
    "        {\n"
    "          \"name\": \"on-stage\",\n"
    "          \"action\": \"play\",\n"
    "          \"animation\": \"Animation_1\"\n"
    "        }\n"
    "      ]\n"
    "    },\n"
    "    {\n"
    "      \"type\":\"CameraActor\",\n"
    "      \"name\":\"fbCam\",\n"
    "      \"aspect-ratio\": \"{FB_ASPECT_RATIO}\",\n"
    "      \"projection-mode\": \"PERSPECTIVE_PROJECTION\",\n"
    "      \"field-of-view\": 0.785,\n"
    "      \"invert-y-axis\": true\n"
    "    }\n"
    "  ],\n"
    "  \"frame-buffer-images\":\n"
    "  {\n"
    "    \"fb0\":\n"
    "    {\n"
    "      \"type\": \"FrameBufferImage\",\n"
    "      \"width\": { \"type-cast\":\"float\", \"value\":\"{FB_WIDTH}\" },\n"
    "      \"height\": { \"type-cast\":\"float\", \"value\":\"{FB_HEIGHT}\" }\n"
    "    }\n"
    "  },\n"
    "  \"render-tasks\":\n"
    "  {\n"
    "    \"stage\":\n"
    "    [\n"
    "      {\n"
    "        \"source-actor\": \"fbOnStage\"\n"
    "      },\n"
    "      {\n"
    "        \"source-actor\": \"Image1\",\n"
    "        \"target-frame-buffer\": \"fb0\",\n"
    "        \"viewport-size\":\"{FB_SIZE}\",\n"
    "        \"camera-actor\":\"fbCam\"\n"
    "      }\n"
    "    ]\n"
    "  },\n"
    "  \"paths\": {},\n"
    "  \"animations\": {\n"
    "    \"Animation_1\": {\n"
    "      \"loop\":true,\n"
    "      \"properties\": [\n"
    "        {\n"
    "          \"actor\": \"Image1\",\n"
    "          \"property\": \"uTime\",\n"
    "          \"value\": 10.0,\n"
    "          \"alpha-function\": \"LINEAR\",\n"
    "          \"time-period\": {\n"
    "            \"delay\": 0,\n"
    "            \"duration\": 10.0\n"
    "          },\n"
    "          \"gui-builder-timeline-color\": \"#8dc0da\"\n"
    "        }\n"
    "      ]\n"
    "    }\n"
    "  },\n"
    "  \"shader-effects\": {\n"
    "    \"Ripple2D\": {\n"
    "      \"program\": {\n"
    "        \"vertexPrefix\": \"\",\n"
    "        \"vertex\": \"void main(void)\\n{\\n  gl_Position = uProjection * uModelView * vec4(aPosition, 1.0);\\n  vTexCoord = aTexCoord;\\n}\\n\\n\",\n"
    "        \"fragmentPrefix\": \"\",\n"
    "        \"fragment\": \"precision mediump float;\\nuniform float uAmplitude; // 0.02; (< 1)\\nuniform float uTime;\\nvoid main()\\n{\\n  highp vec2 textureSize = sTextureRect.zw - sTextureRect.xy;\\n  highp vec2 pos = -1.0 + 2.0 * vTexCoord.st/textureSize;\\n  highp float len = length(pos);\\n  highp vec2 texCoord = vTexCoord.st/textureSize + pos/len * sin( len * 12.0 - uTime * 4.0 ) * uAmplitude; \\n  gl_FragColor = texture2D(sTexture, texCoord) * uColor;\\n}\\n\\n\\n\",\n"
    "        \"geometry-type\": \"GEOMETRY_TYPE_IMAGE\"\n"
    "      },\n"
    "      \"geometry-hints\": \"HINT_NONE\",\n"
    "      \"grid-density\": 0,\n"
    "      \"loop\": true,\n"
    "      \"uAmplitude\": 0.02,\n"
    "      \"uTime\": 0.0\n"
    "    }\n"
    "  }\n"
    "}\n");

  Builder builder = Builder::New();

  // frame buffer coverage
  builder.LoadFromString( json );

  // Render and notify
  application.SendNotification();
  application.Render();

  Dali::FrameBufferImage frameBuffer = builder.GetFrameBufferImage( "fb0" );
  DALI_TEST_CHECK( frameBuffer );

  Dali::FrameBufferImage frameBuffer2 = builder.GetFrameBufferImage( "fb0" );
  DALI_TEST_CHECK( frameBuffer2 );
  DALI_TEST_CHECK( frameBuffer == frameBuffer2 );

  DALI_TEST_CHECK( true );

  END_TEST;
}

int UtcDaliBuilderPathConstraintsP(void)
{
  ToolkitTestApplication application;

  // JSON with a quit event when the actor is touched
  std::string json(
    "{\n"
    "  \"constants\":\n"
    "  {\n"
    "    \"FB_WIDTH\": 200.0,\n"
    "    \"FB_HEIGHT\": 200.0,\n"
    "    \"FB_SIZE\": [200,200],\n"
    "    \"FB_ASPECT_RATIO\": 1\n"
    "  },\n"
    "  \"stage\": [\n"
    "    {\n"
    "      \"type\": \"ImageActor\",\n"
    "      \"name\": \"Image1\",\n"
    "      \"size\": [200, 200, 0],\n"
    "      \"parent-origin\": [0.5, 0.5, 0.5],\n"
    "      \"effect\": \"Ripple2D\",\n"
    "      \"image\": {\n"
    "        \"filename\": \"{DALI_IMAGE_DIR}gallery-medium-25.jpg\",\n"
    "        \"width\": 200,\n"
    "        \"height\": 80,\n"
    "        \"load-policy\": \"IMMEDIATE\",\n"
    "        \"release-policy\": \"NEVER\"\n"
    "      },\n"
    "      \"signals\": [\n"
    "        {\n"
    "          \"name\": \"on-stage\",\n"
    "          \"action\": \"play\",\n"
    "          \"animation\": \"path-animation\"\n"
    "        },\n"
    "        {\n"
    "          \"name\": \"on-stage\",\n"
    "          \"action\": \"applyConstraint\",\n"
    "          \"constrainer\": \"constrainer0\",\n"
    "          \"properties\":\n"
    "          [\n"
    "            {\n"
    "              \"source\": \"Image1\",\n"
    "              \"sourceProperty\": \"position-x\",\n"
    "              \"target\": \"Image1\",\n"
    "              \"targetProperty\": \"color-red\",\n"
    "              \"range\": [-300,300]\n"
    "            }\n"
    "          ]\n"
    "        },\n"
    "        {\n"
    "          \"name\": \"on-stage\",\n"
    "          \"action\": \"applyConstraint\",\n"
    "          \"constrainer\": \"constrainer1\",\n"
    "          \"properties\":\n"
    "          [\n"
    "            {\n"
    "              \"source\": \"Image1\",\n"
    "              \"sourceProperty\": \"position-x\",\n"
    "              \"target\": \"Image1\",\n"
    "              \"targetProperty\": \"color-blue\",\n"
    "              \"range\": [-300,300]\n"
    "            }\n"
    "          ]\n"
    "        }\n"
    "      ]\n"
    "    }\n"
    "  ],\n"
    "  \"paths\":\n"
    "  {\n"
    "    \"path0\":\n"
    "    {\n"
    "      \"points\":[ [-150, -50, 0], [0.0,70.0,0.0], [190.0,-150.0,0.0] ],\n"
    "      \"curvature\":0.35\n"
    "    }\n"
    "  },\n"
    "  \"constrainers\":\n"
    "  {\n"
    "    \"constrainer0\":\n"
    "    {\n"
    "      \"type\": \"PathConstrainer\",\n"
    "      \"points\": [ [0, 0, 0], [0,0,0], [0,0,0] ],\n"
    "      \"control-points\": [ [0, 0, 0], [0,0,0], [0,0,0] ]\n"
    "    },\n"
    "    \"constrainer1\":\n"
    "    {\n"
    "      \"type\": \"LinearConstrainer\",\n"
    "      \"value\": [ 0, 0, 0 ]\n"
    "    }\n"
    "  },\n"
    "  \"animations\": {\n"
    "    \"path-animation\": {\n"
    "      \"duration\": 3.0,\n"
    "      \"properties\":\n"
    "      [{\n"
    "        \"actor\": \"Image1\",\n"
    "        \"path\":\"path0\",\n"
    "        \"forward\":[1,0,0],\n"
    "        \"alpha-function\": \"EASE_IN_OUT\",\n"
    "        \"time-period\": {\n"
    "          \"delay\": 0,\n"
    "          \"duration\": 3\n"
    "        }\n"
    "      },\n"
    "       {\n"
    "         \"actor\": \"Image1\",\n"
    "         \"property\": \"uTime\",\n"
    "         \"value\": 10.0,\n"
    "         \"alpha-function\": \"LINEAR\",\n"
    "         \"time-period\": {\n"
    "           \"delay\": 0,\n"
    "           \"duration\": 10.0\n"
    "         },\n"
    "         \"gui-builder-timeline-color\": \"#8dc0da\"\n"
    "       }]\n"
    "    },\n"
    "    \"Animation_1\": {\n"
    "      \"loop\":true,\n"
    "      \"properties\": [\n"
    "        {\n"
    "          \"actor\": \"Image1\",\n"
    "          \"property\": \"uTime\",\n"
    "          \"value\": 10.0,\n"
    "          \"alpha-function\": \"LINEAR\",\n"
    "          \"time-period\": {\n"
    "            \"delay\": 0,\n"
    "            \"duration\": 10.0\n"
    "          },\n"
    "          \"gui-builder-timeline-color\": \"#8dc0da\"\n"
    "        }\n"
    "      ]\n"
    "    }\n"
    "  },\n"
    "  \"shader-effects\": {\n"
    "    \"Ripple2D\": {\n"
    "      \"program\": {\n"
    "        \"vertexPrefix\": \"\",\n"
    "        \"vertex\": \"void main(void)\\n{\\n  gl_Position = uProjection * uModelView * vec4(aPosition, 1.0);\\n  vTexCoord = aTexCoord;\\n}\\n\\n\",\n"
    "        \"fragmentPrefix\": \"\",\n"
    "        \"fragment\": \"precision mediump float;\\nuniform float uAmplitude; // 0.02; (< 1)\\nuniform float uTime;\\nvoid main()\\n{\\n  highp vec2 textureSize = sTextureRect.zw - sTextureRect.xy;\\n  highp vec2 pos = -1.0 + 2.0 * vTexCoord.st/textureSize;\\n  highp float len = length(pos);\\n  highp vec2 texCoord = vTexCoord.st/textureSize + pos/len * sin( len * 12.0 - uTime * 4.0 ) * uAmplitude; \\n  gl_FragColor = texture2D(sTexture, texCoord) * uColor;\\n}\\n\\n\\n\",\n"
    "        \"geometry-type\": \"GEOMETRY_TYPE_IMAGE\"\n"
    "      },\n"
    "      \"geometry-hints\": \"HINT_NONE\",\n"
    "      \"grid-density\": 0,\n"
    "      \"loop\": true,\n"
    "      \"uAmplitude\": 0.02,\n"
    "      \"uTime\": 0.0\n"
    "    }\n"
    "  }\n"
    "}\n");

  Builder builder = Builder::New();

  // frame buffer coverage
  builder.LoadFromString( json );

  // Render and notify
  application.SendNotification();
  application.Render();

  Dali::Path path =  builder.GetPath( "path0" );
  DALI_TEST_CHECK( path );

  Dali::Path path2 =  builder.GetPath( "path0" );
  DALI_TEST_CHECK( path2 );
  DALI_TEST_CHECK( path == path2 );

  Dali::PathConstrainer constrainer0 = builder.GetPathConstrainer( "constrainer0" );
  DALI_TEST_CHECK( constrainer0 );

  Dali::PathConstrainer constrainer0_2 = builder.GetPathConstrainer( "constrainer0" );
  DALI_TEST_CHECK( constrainer0_2 );
  DALI_TEST_CHECK( constrainer0 == constrainer0_2 );

  Dali::LinearConstrainer constrainer1 = builder.GetLinearConstrainer( "constrainer1" );
  DALI_TEST_CHECK( constrainer1 );

  Dali::LinearConstrainer constrainer1_2 = builder.GetLinearConstrainer( "constrainer1" );
  DALI_TEST_CHECK( constrainer1 == constrainer1_2 );

  END_TEST;
}
