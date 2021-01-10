#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <memory>
#include <iostream>
//#include "logger.h"
#include <chrono>
#include <QDebug>

//#define CoreQueueLog ILOGW("_CORE_Queue")
#define CoreQueueLog qDebug()<<"_CORE_Queue"
namespace ifly
{
    namespace core
    {
        template <class T>
        class ThreadMsgQueuePtr
        {
        private:
            /* data */
            std::thread looper;
            std::mutex queue_mutex;
            std::mutex exit_mutex;

            bool stop;
            std::condition_variable condition;
            std::condition_variable exit_condition;

            std::deque<std::shared_ptr<T>> queue;
            void dispatchMessage(std::shared_ptr<T> message);
            bool loopStoped = false;

        public:
            ThreadMsgQueuePtr(/* args */);
            ~ThreadMsgQueuePtr();
            void sendMessage(std::shared_ptr<T> message);
            void removeMessage(std::shared_ptr<T> message);
            void removeAllMessage();
            virtual void handleMessage(std::shared_ptr<T> message){};
            virtual void onRemoveMessage(std::shared_ptr<T> message){};

            void loop();
        };

        template <class T>
        ThreadMsgQueuePtr<T>::ThreadMsgQueuePtr(/* args */)
        {
            stop = false;
            looper = std::thread(&ThreadMsgQueuePtr::loop, this);
            CoreQueueLog << "ThreadMsgQueuePtr.";
        }

        template <class T>
        ThreadMsgQueuePtr<T>::~ThreadMsgQueuePtr()
        {
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                stop = true;
            }

            condition.notify_all();
            {
                CoreQueueLog << "~ThreadMsgQueuePtr: lock.";
                std::unique_lock<std::mutex> exitLock(this->exit_mutex);
                CoreQueueLog << "~ThreadMsgQueuePtr: get lock will condition wait.";
                exit_condition.wait_for(exitLock, std::chrono::milliseconds(100), [this] { return (loopStoped); }); //如果没有退出条件判断，可能会导致loop迟于析构退出，从而导致有possibly的泄露，但实际上没影响。
                CoreQueueLog << "~ThreadMsgQueuePtr: exit_condition ok.";
                looper.detach();
                queue.clear();
                looper.~thread();
                CoreQueueLog << "~ThreadMsgQueuePtr: end.";
            }
        }

        template <class T>
        void ThreadMsgQueuePtr<T>::sendMessage(std::shared_ptr<T> message)
        {

            std::unique_lock<std::mutex> lock(queue_mutex);
            typename std::deque<std::shared_ptr<T>>::iterator it;
            for (it = queue.begin(); it != queue.end(); ++it)
            {
                if (**it == *message)
                {
                    CoreQueueLog << "sendMessage: message is already in queue, return.";
                    return;
                }
            }

            queue.push_back(message);

            condition.notify_one();
            CoreQueueLog << "sendMessage: after size = " << queue.size();
            return;
        }

        template <class T>
        void ThreadMsgQueuePtr<T>::removeAllMessage()
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            queue.clear();
        }

        template <class T>
        void ThreadMsgQueuePtr<T>::removeMessage(std::shared_ptr<T> message)
        {

            std::unique_lock<std::mutex> lock(queue_mutex);
            typename std::deque<std::shared_ptr<T>>::iterator it;
            for (it = queue.begin(); it != queue.end(); ++it)
            {
                if (**it == *message)
                {
                    CoreQueueLog << "removeMessage: message is found in queue, erase it now.";
                    std::shared_ptr<T> value = *it;
                    queue.erase(it);
                    onRemoveMessage(value);

                    break;
                }
            }
            CoreQueueLog << "removeMessage: after size = " << queue.size();
            condition.notify_one();
        }
        template <class T>

        void ThreadMsgQueuePtr<T>::dispatchMessage(std::shared_ptr<T> message)
        {
            handleMessage(message);
        }

        template <class T>
        void ThreadMsgQueuePtr<T>::loop()
        {
            for (;;)
            {
                std::shared_ptr<T> t;
                bool hasMessge = false;
                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    condition.wait(lock, [this] { return (stop || !queue.empty()); });

                    if (this->stop)
                    {
                        queue.clear();
                        CoreQueueLog << "loop will stop.";
                        {
                            std::unique_lock<std::mutex> exitLock(this->exit_mutex);
                            CoreQueueLog << "loop exitLock.";

                            loopStoped = true;
                            exit_condition.notify_all();
                        }
                        CoreQueueLog << "loop out exitLock.";

                        return;
                    }

                    if (!queue.empty())
                    {
                        t = queue.front();
                        queue.pop_front();
                        hasMessge = true;
                    }
                }
                if (hasMessge)
                {
                    this->dispatchMessage(t);
                }
            }
        }

    } // namespace core
} // namespace ifly
