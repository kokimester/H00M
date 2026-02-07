#pragma once

#include <vector>

class Subscriber;

class Observer {
private:
  // assumes no ownership of the object
  std::vector<Subscriber*> m_subscribers;

public:
  virtual ~Observer() = default;
  virtual void subscribe(Subscriber*);
  virtual void unsubscribe(Subscriber*);
  virtual void notify_subscribers();
  virtual void update() = 0;
};
