#include "Action.hpp"
#include "ActionSets.hpp"
#include "BasicActions.hpp"

#include "JsonSerializers.hpp"

struct Context
{
  std::shared_ptr<ActionDefinitions> mDefinitions;
};

void ParseSequence(JsonLoader& loader, ActionSet& parent, Context& context);
void ParseGroup(JsonLoader& loader, ActionSet& parent, Context& context);

void ParseSet(JsonLoader& loader, ActionSet& set, Context& context)
{
  size_t count;
  if(loader.BeginArray(count))
  {
    for(size_t i = 0; i < count; ++i)
    {
      loader.BeginArrayItem(i);

      String name;
      if(loader.BeginMember("Name"))
      {
        loader.SerializePrimitive(name);
        loader.EndMember();
      }

      if(loader.BeginMember("Delay"))
      {
        float delay = 0.0f;
        loader.SerializePrimitive(delay);
        set.Add<ActionDelay>(delay);
        loader.EndMember();
      }
      else if(loader.BeginMember("Print"))
      {
        std::string message;
        loader.SerializePrimitive(message);
        set.Add<ActionPrint>(message);
        loader.EndMember();
      }
      else if(loader.BeginMember("Sequence"))
      {
        ParseSequence(loader, set, context);
        loader.EndMember();
      }
      else if(loader.BeginMember("Group"))
      {
        ParseGroup(loader, set, context);
        loader.EndMember();
      }
      else if(loader.BeginMember("Definition"))
      {
        std::string definitionName;
        //if(loader.BeginMember("DefinitionName"))
        //{
          loader.SerializePrimitive(definitionName);
          set.Add<ActionCreateDefinition>(context.mDefinitions, definitionName);
        //  loader.EndMember();
        //}
        loader.EndMember();
      }

      loader.EndArrayItem();
    }
  }
}

void ParseSequence(JsonLoader& loader, ActionSet& parent, Context& context)
{
  auto seq = parent.AddAndGet<ActionSequence>();
  ParseSet(loader, *seq, context);
}

void ParseGroup(JsonLoader& loader, ActionSet& parent, Context& context)
{
  auto group = parent.AddAndGet<ActionGroup>();
  ParseSet(loader, *group, context);
}

void ParseDefinitions(JsonLoader& loader, Context& context)
{
  size_t memberCount;
  if(loader.BeginMembers(memberCount))
  {
    for(size_t i = 0; i < memberCount; ++i)
    {
      String memberName;
      if(loader.BeginMember(i, memberName))
      {

        if(loader.BeginMember("Sequence"))
        {
          auto seq = std::make_shared<ActionSequence>();
          ParseSet(loader, *seq, context);

          context.mDefinitions->mMap[memberName] = seq;
          loader.EndMember();
        }
        else if(loader.BeginMember("Group"))
        {
          auto group = std::make_shared<ActionGroup>();
          ParseSet(loader, *group, context);

          context.mDefinitions->mMap[memberName] = group;
          loader.EndMember();
        }
        loader.EndMember();
      }
    }
    loader.EndMembers();
  }
}

void ParseActions(const std::string& filePath, ActionGroup& root, Context& context)
{
  JsonLoader loader;
  loader.LoadFromFile(filePath);

  if(loader.BeginMember("Root"))
  {
    ParseGroup(loader, root, context);
    loader.EndMember();
  }
  if(loader.BeginMember("Definitions"))
  {
    ParseDefinitions(loader, context);
    loader.EndMember();
  }
}

void TestJson()
{
  Context context;
  context.mDefinitions = std::make_shared<ActionDefinitions>();
  float dt = 0.016f;

  ActionGroup root;
  ParseActions("Data/Actions.json", root, context);
  float totalTime = 0;
  while(root.Execute(dt) != ActionState::Finished)
  {
    totalTime += dt;
    printf("Total time: %f\n", totalTime);
  }
}

int main()
{
  TestJson();
  float dt = 0.016f;

  ActionGroup root;
  float totalTime = 0;

  auto seq = std::make_shared<ActionSequence>();
  auto group1 = std::make_shared<ActionGroup>();
  root.mChildActions.push_back(seq);

  seq->mChildActions.push_back(group1);
  group1->Add<ActionDelay>(1.0f);
  auto delay1 = group1->AddAndGet<ActionDelay>(1.5f);
  group1->Add<ActionPrint>("Group 1 started\n");

  seq->Add<ActionPrint>("Group 1 finished\n");
  seq->Add<ActionDelay>(1.0f);
  seq->Add<ActionPrint>("Print1\n");
  seq->Add<ActionDelay>(1.5f);
  seq->Add<ActionPrint>("Sequence Finished\n");
  seq->Add<ActionCall>([](float dt) -> ActionState
  {
    printf("Callback\n");
    return ActionState::Finished;
  });
  root.Add<ActionPrint>("Print3\n");

  totalTime = 0;
  while(root.Execute(dt) != ActionState::Finished)
  {
    if(delay1->mState != ActionState::Paused)
      delay1->Pause();
    if(totalTime >= 5 && delay1->mState == ActionState::Paused)
      delay1->Resume();

    totalTime += dt;
    printf("Total time: %f\n", totalTime);
  }

  return 0;
}
