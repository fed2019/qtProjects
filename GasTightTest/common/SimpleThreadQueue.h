#pragma once

#include "ThreadMsgQueuePtr.h"
#include <functional>

namespace ifly
{

    namespace core
    {
        class Runable
        {
        public:
            Runable(int64_t id)
            {
                mId = id;
            }
            ~Runable()
            {
            }

            std::function<void()> proc;

            bool operator==(const Runable &runable)
            {
                return this->mId == runable.mId;
            }

        private:
            int64_t mId;
        };

        class SimpleThreadQueue : private core::ThreadMsgQueuePtr<Runable>
        {
        public:
            SimpleThreadQueue()
            {
            }
            ~SimpleThreadQueue()
            {
            }

            template <typename F, typename... Args>
            auto post(F fun, Args &&... args)
            {
                std::unique_lock<std::mutex> lock(mMutex);
                mCount++;
                auto runable = std::make_shared<Runable>(mCount);
                runable->proc = [=]() {
                    fun(args...);
                };
                sendMessage(runable);
                return runable;
            }

            template <typename F>
            auto post(F fun)
            {
                std::unique_lock<std::mutex> lock(mMutex);
                mCount++;
                auto runable = std::make_shared<Runable>(mCount);
                runable->proc = fun;
                sendMessage(runable);
                return runable;
            }

            void removeCallbacks(std::shared_ptr<Runable> action)
            {
                removeMessage(action);
            }

            void removeAllCallbacks()
            {
                removeAllMessage();
            }

        private:
            std::mutex mMutex;
            int64_t mCount = 0;
            void virtual handleMessage(std::shared_ptr<Runable> action) override
            {
                action->proc();
            }
            void virtual onRemoveMessage(std::shared_ptr<Runable> action) override
            {
            }
        };

    } // namespace core

} // namespace ifly
