#pragma once

#include "Action.hpp"

class ActionSet : public Action
{
public:
  
  template <typename T, typename...Args>
  std::shared_ptr<T> AddAndGet(Args...args)
  {
    std::shared_ptr<T> result = std::make_shared<T>(args...);
    mChildActions.push_back(result);
    result->mParent = this;
    return result;
  }

  template <typename T, typename...Args>
  void Add(Args...args)
  {
    std::shared_ptr<T> result = AddAndGet<T>(args...);
  }

  virtual void CopyPropertiesTo(ActionSet& set)
  {
    for(auto&& child : mChildActions)
      child->CloneTo(set);
  }

  std::vector<SharedAction> mChildActions;
};

class ActionSequence : public ActionSet
{
public:
  virtual ActionState OnExecute(float dt) override
  {
    if(mChildActions.empty())
      return ActionState::Finished;

    ActionState childState = mChildActions.front()->Execute(dt);
    if(childState == ActionState::Finished || childState == ActionState::Cancelled)
      mChildActions.erase(mChildActions.begin());

    if(mChildActions.empty())
      return ActionState::Finished;
    return ActionState::Running;
  }
  virtual void CloneTo(ActionSet& parent) override
  {
    auto clone = parent.AddAndGet<ActionSequence>();
    clone->mState = mState;
    CopyPropertiesTo(*clone);
  }
};

class ActionGroup : public ActionSet
{
public:
  virtual ActionState OnExecute(float dt) override
  {
    auto it = mChildActions.begin();
    while(it != mChildActions.end())
    {
      ActionState childState = (*it)->Execute(dt);
      if(childState == ActionState::Finished || childState == ActionState::Cancelled)
        it = mChildActions.erase(it);
      else
        ++it;
    }

    if(mChildActions.empty())
      return ActionState::Finished;
    return ActionState::Running;
  }
  virtual void CloneTo(ActionSet& parent) override
  {
    auto clone = parent.AddAndGet<ActionGroup>();
    clone->mState = mState;
    CopyPropertiesTo(*clone);
  }
};
