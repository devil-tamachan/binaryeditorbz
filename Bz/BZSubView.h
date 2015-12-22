#pragma once

class CBZSubView
{
public:

  CBZSubView(void)
  {
  }

  virtual ~CBZSubView(void)
  {
  }

  virtual int GetWindowIdealWidth() = 0;

  virtual void InitSubView() = 0;
};

