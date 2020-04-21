#pragma once

#include <vector>
#include <memory>

class Action;
class ActionSet;
typedef std::shared_ptr<Action> SharedAction;

enum class ActionState
{
  Running,
  Finished,
  Cancelled,
  Paused
};

class Action
{
public:
  virtual ActionState Execute(float dt)
  {
    if(mState != ActionState::Running)
      return mState;
    return OnExecute(dt);
  }
  virtual ActionState OnExecute(float dt) abstract;
  virtual void CloneTo(ActionSet& parent) abstract;
  //virtual void CopyPropertiesTo(Action& action)
  //{
  //  action.mState = mState;
  //}

  virtual bool Cancel()
  {
    if(mState == ActionState::Running || mState == ActionState::Paused)
      mState = ActionState::Cancelled;
    return true;
  }
  virtual bool Resume()
  {
    if(mState == ActionState::Paused)
      mState = ActionState::Running; 
    return true;
  }
  virtual bool Pause()
  {
    if(mState == ActionState::Running)
      mState = ActionState::Paused;
    return true;
  }

  ActionSet* mParent = nullptr;
  ActionState mState = ActionState::Running;
};
