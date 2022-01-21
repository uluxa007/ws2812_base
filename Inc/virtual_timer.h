#ifndef COMMON_VIRTUAL_TIMER_H_
#define COMMON_VIRTUAL_TIMER_H_


#include <atomic>
#include <cstddef>
#include <etl/vector.h>


namespace Common
{
    
    class IVirtualTimerListener;
    
    class IVirtualTimer
    {
    public:
        using TimerId = size_t;        
        
        virtual ~IVirtualTimer() = default;
        
        virtual TimerId GetId() const = 0;
        
        virtual void Start(size_t msec) = 0;

        virtual void Stop() = 0;
        
        virtual void Restart() = 0;
        
        virtual bool IsEnabled() const = 0;
        
        virtual bool IsOccurred() const = 0;
        
        virtual void Subscribe(IVirtualTimerListener& listener) = 0;
        
        virtual void Unsubscribe() = 0;
    };
    
    class IVirtualTimerListener
    {
    public:
        virtual ~IVirtualTimerListener() = default;
            
        virtual void OnTimer(IVirtualTimer& timer) = 0;
    };
    
    class IVirtualTimerFactory
    {
    public:
        virtual ~IVirtualTimerFactory() = default;
        
        virtual IVirtualTimer& CreateTimer(size_t msec = 0) = 0;

        // Creates timer, which triggers subscriber's callback inside Decrement method
        // It highly increases a timer accuracy, but you should create such a timer
        // for super fast methods only!
        virtual IVirtualTimer& CreateFineTimer(size_t msec = 0) = 0;
        
        virtual void Decrement(size_t msec) = 0;
        
        virtual void NotifySubscribers() = 0;
    };
    
    template<size_t Capacity>
    class VirtualTimerFactory : public IVirtualTimerFactory
    {
    public:
        VirtualTimerFactory() = default;
        
        VirtualTimerFactory(const VirtualTimerFactory&) = delete;
        
        VirtualTimerFactory& operator=(const VirtualTimerFactory&) = delete;
        
        IVirtualTimer& CreateTimer(size_t msec = 0) override
        {
            m_timers.emplace_back(m_timers.size(), msec, false);
            return m_timers.back();
        }

        IVirtualTimer& CreateFineTimer(size_t msec = 0) override
        {
            m_timers.emplace_back(m_timers.size(), msec, true);
            return m_timers.back();
        }
        
        void Decrement(size_t msec) override
        {
            for (auto& timer : m_timers)
            {
                if (!timer.enabled)
                    continue;
                
                auto counter = timer.counter.load();
                auto new_counter = counter > msec ? counter - msec : 0;
                timer.counter.store(new_counter);

                if (new_counter == 0 && timer.lightweight && timer.listener != nullptr)
                {
                    timer.listener->OnTimer(timer);
                }
            }
        }
        
        void NotifySubscribers() override
        {
            for (auto& timer : m_timers)
            {
                if (timer.IsOccurred() && timer.listener != nullptr && !timer.lightweight)
                {
                    timer.listener->OnTimer(timer);
                }
            }
        }        
        
    private:
        struct VirtualTimer : public IVirtualTimer
        {
            VirtualTimer(IVirtualTimer::TimerId id_, size_t timeout_, bool lightweight_)
                : id(id_)
                , timeout(timeout_)
                , enabled(false)
                , counter(0)
                , lightweight(lightweight_)
                , listener(nullptr)
            {
            }
            
            TimerId GetId() const override
            {
                return id;
            }
            
            bool IsEnabled() const override
            {
                return enabled;
            }
            
            bool IsOccurred() const override
            {
                return enabled && counter == 0;
            }            
            
            void Restart() override
            {
                Start(timeout);
            }             
            
            void Start(size_t msec) override
            {
                timeout = msec;
                counter.store(msec);
                enabled = true;                
            }            
            
            void Stop() override
            {
                enabled = false;
            }            
            
            void Subscribe(IVirtualTimerListener& listener_) override
            {
                listener = &listener_;
            }
            
            void Unsubscribe() override
            {
                listener = nullptr;
            }
            
            IVirtualTimer::TimerId id;
            size_t timeout;
            bool enabled;
            std::atomic_size_t counter;
            bool lightweight;
            IVirtualTimerListener* listener;
        };        
        
        etl::vector<VirtualTimer, Capacity> m_timers;
    };

} // namespace Common


#endif // COMMON_VIRTUAL_TIMER_H_
