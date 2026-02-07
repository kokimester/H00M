#include "Observer.h"
#include "Subscriber.h"
#include <algorithm>

void Observer::subscribe(Subscriber* s) {
  auto itr = std::find(m_subscribers.begin(), m_subscribers.end(), s);
  if (itr == m_subscribers.end()) {
    m_subscribers.push_back(s);
  }
}

void Observer::unsubscribe(Subscriber* s) {
  auto itr = std::find(m_subscribers.begin(), m_subscribers.end(), s);
  if (itr != m_subscribers.end()) {
    m_subscribers.erase(
        std::remove(m_subscribers.begin(), m_subscribers.end(), s));
  }
}

void Observer::notify_subscribers() {
  std::for_each(m_subscribers.begin(), m_subscribers.end(),
                [](auto& s) { s->notify(); });
}
