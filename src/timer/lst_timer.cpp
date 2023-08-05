#include "lst_timer.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

sort_timer_lst::sort_timer_lst()
{
    head = NULL;
    tail = NULL;
}

sort_timer_lst::~sort_timer_lst()
{
    util_timer *tmp = head;
    while (tmp)
    {
        head = tmp->next;
        delete tmp;
        tmp = head;
    }
}

void sort_timer_lst::add_timer(util_timer *timer)
{
    if (!timer)
    {
        return;
    }

    m_lock.lock();
    if (ref.count(timer) == 0)
    {
        // 新节点
        if (!head)
        {
            head = tail = timer;
            ref.insert(timer);
            m_lock.unlock();
            return;
        }
        if (timer->expire < head->expire)
        {
            timer->next = head;
            head->prev = timer;
            head = timer;
            ref.insert(timer);
            m_lock.unlock();
            return;
        }
        add_timer(timer, head);
        ref.insert(timer);
        m_lock.unlock();
        return;
    }
    else
    {
        // 已有节点, 只调整位置
        m_lock.unlock();
        adjust_timer(timer);
    }
}

void sort_timer_lst::adjust_timer(util_timer *timer)
{
    if (!timer)
    {
        return;
    }

    m_lock.lock();
    assert(ref.count(timer) > 0);
    util_timer *tmp = timer->next;
    if (!tmp || (timer->expire < tmp->expire))
    {
        m_lock.unlock();
        return;
    }
    if (timer == head)
    {
        head = head->next;
        head->prev = NULL;
        timer->next = NULL;
        add_timer(timer, head);
    }
    else
    {
        timer->prev->next = timer->next;
        timer->next->prev = timer->prev;
        add_timer(timer, timer->next);
    }
    m_lock.unlock();
}

void sort_timer_lst::del_timer(util_timer *timer)
{
    if (!timer)
    {
        return;
    }

    m_lock.lock();
    if (ref.count(timer) == 0)
    {
        m_lock.unlock();
        return;
    }
    if ((timer == head) && (timer == tail))
    {
        delete timer;
        ref.erase(timer);
        head = NULL;
        tail = NULL;
        m_lock.unlock();
        return;
    }
    if (timer == head)
    {
        head = head->next;
        head->prev = NULL;
        delete timer;
        ref.erase(timer);
        m_lock.unlock();
        return;
    }
    if (timer == tail)
    {
        tail = tail->prev;
        tail->next = NULL;
        delete timer;
        ref.erase(timer);
        m_lock.unlock();
        return;
    }
    timer->prev->next = timer->next;
    timer->next->prev = timer->prev;
    delete timer;
    ref.erase(timer);
    m_lock.unlock();
}

void sort_timer_lst::tick(int epollfd)
{
    m_lock.lock();
    if (!head)
    {
        m_lock.unlock();
        return;
    }
    time_t cur = time(NULL);
    util_timer *tmp = head;
    while (tmp)
    {
        if (cur < tmp->expire)
        {
            break;
        }
        tmp->cb_func(epollfd, tmp->user_data);
        m_lock.unlock();
        del_timer(tmp);
        m_lock.lock();
        tmp = head;
    }
    m_lock.unlock();
}

void sort_timer_lst::add_timer(util_timer *timer, util_timer *lst_head)
{
    util_timer *prev = lst_head;
    util_timer *tmp = prev->next;
    while (tmp)
    {
        if (timer->expire < tmp->expire)
        {
            prev->next = timer;
            timer->next = tmp;
            tmp->prev = timer;
            timer->prev = prev;
            break;
        }
        prev = tmp;
        tmp = tmp->next;
    }
    if (!tmp)
    {
        prev->next = timer;
        timer->prev = prev;
        timer->next = NULL;
        tail = timer;
    }
}
