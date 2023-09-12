#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.
    std::unique_lock<std::mutex> lck(_mutex);
    _cond.wait(lck, [this] {return !_queue.empty();});
    T message = std::move(_queue.back());
    return message;
}

template <typename T>
void MessageQueue<T>::send(T&& msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a
    // notification.
    std::lock_guard<std::mutex> lck(_mutex);
    _queue.emplace_back(std::move(msg));
    _cond.notify_one();
}

/* Implementation of class "TrafficLight" */ 
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true) {
        TrafficLightPhase light_color = _msgQueue.receive();
        if (light_color == green) {
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method "cycleThroughPhases" should be started in a thread when
    // the public method "simulate" is called. To do this, use the thread queue in the base class.
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop
    // cycles and toggles the current phase of the traffic light between red and green and sends an
    // update method to the message queue using move semantics. The cycle duration should be a
    // random value between 4 and 6 seconds. Also, the while-loop should use std::this_thread::sleep
    // for to wait 1ms between two cycles.

    // Get random light duration between 4 and 6
    // Set the seed for each instance of the TrafficLight. If this isn't there, all lights change simultaneously
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine gen(seed);
    std::uniform_real_distribution<double> distribution(4.0, 6.0);
    double light_duration = distribution(gen);

    // Get start time
    auto t0 = std::chrono::system_clock::now();

    while(true) {
        // Get elapsed time
        auto t1 = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = t1 - t0;

        if (elapsed_seconds.count() > light_duration) {
            // Change phase
            _currentPhase = (_currentPhase == red) ? green : red;

            // Update message queue
            TrafficLightPhase cur_phase = _currentPhase;
            _msgQueue.send(std::move(cur_phase));

            // Pick new random time and reset start time
            light_duration = distribution(gen);
            t0 = std::chrono::system_clock::now();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

