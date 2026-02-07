#pragma once

class Subscriber {
public:
  virtual ~Subscriber() = default;
  virtual void notify() = 0;
};
