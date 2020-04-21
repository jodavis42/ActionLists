#pragma once

#include "Action.hpp"
#include "ActionSets.hpp"
#include <string>
#include <functional>

class ActionDelay : public Action
{
public:
  ActionDelay(float delay) : mDelay(delay)
  {

  }
  virtual ActionState OnExecute(float dt) override
  {
    mElapsedTime += dt;
    if(mElapsedTime >= mDelay)
      return ActionState::Finished;
    return ActionState::Running;
  }
  virtual void CloneTo(ActionSet& parent) override
  {
    auto clone = parent.AddAndGet<ActionDelay>(mDelay);
    clone->mState = mState;
  }

  float mElapsedTime = 0;
  float mDelay = 1.0f;
};

class ActionCall : public Action
{
public:
  typedef std::function<ActionState(float)> FunctionType;
  ActionCall(FunctionType fn) : mFunction(fn)
  {

  }
  virtual ActionState OnExecute(float dt) override
  {
    if(mFunction == nullptr)
      return ActionState::Finished;

    return mFunction(dt);
  }
  virtual void CloneTo(ActionSet& parent) override
  {
    auto clone = parent.AddAndGet<ActionCall>(mFunction);
    clone->mState = mState;
  }

  FunctionType mFunction;
};

class ActionPrint : public Action
{
public:
  ActionPrint(const std::string& msg) : mMessage(msg)
  {

  }
  virtual ActionState OnExecute(float dt) override
  {
    printf("%s", mMessage.c_str());
    return ActionState::Finished;
  }
  virtual void CloneTo(ActionSet& parent) override
  {
    auto clone = parent.AddAndGet<ActionPrint>(mMessage);
    clone->mState = mState;
  }

  std::string mMessage;
};

struct ActionDefinitions
{
  std::unordered_map<std::string, SharedAction> mMap;
};


class ActionCreateDefinition : public Action
{
public:
  ActionCreateDefinition(std::shared_ptr<ActionDefinitions> definitions, const std::string& definitionName)
    : mDefinitionName(definitionName)
    , mDefinitions(definitions)
  {

  }
  virtual ActionState OnExecute(float dt) override
  {
    auto it = mDefinitions->mMap.find(mDefinitionName);
    if(it != mDefinitions->mMap.end())
    {
      it->second->CloneTo(*mParent);
    }
    return ActionState::Finished;
  }
  virtual void CloneTo(ActionSet& parent) override
  {
    auto clone = parent.AddAndGet<ActionCreateDefinition>(mDefinitions, mDefinitionName);
    clone->mState = mState;
  }

  std::string mDefinitionName;
  std::shared_ptr<ActionDefinitions> mDefinitions;
};
