#ifndef _LINKLIST_H
#define _LINKLIST_H

#include "common_def.h"

class DoubLink;

struct DoubLinkNode
{
	DoubLink 	 *owner;  
	DoubLinkNode *next;
	DoubLinkNode *prev;

	DoubLinkNode():owner(nullptr),next(nullptr),prev(nullptr)
	{

	}

	DoubLinkNode *Next()
	{
		return next;
	}

	DoubLinkNode *Prev()
	{
		return prev;
	}	


	DoubLinkNode(const DoubLinkNode &o);
	DoubLinkNode& operator = (const DoubLinkNode &o);


	inline ~DoubLinkNode();
};

class DoubLink
{

private:	
	DoubLinkNode head_;
	DoubLinkNode tail_;
	u32     count_;

public:

	u32 Size()
	{
		return count_;
	}

	void Clear()
	{
		DoubLinkNode *pNode = Pop();
		while(nullptr != pNode)
		{
			pNode = Pop();
		}	
	}

	~DoubLink()
	{
		Clear();
	}

	DoubLink():count_(0)
	{
		head_.next = &tail_;
		tail_.prev = &head_;		
	}

	DoubLinkNode *GetFirst()
	{
		return head_.next;
	}

	DoubLinkNode *GetHead()
	{
		return &head_;
	}

	DoubLinkNode *GetTail()
	{
		return &tail_;
	}

	void Push(DoubLinkNode *dNode)
	{

		if((nullptr == dNode->next && (nullptr != dNode->prev || nullptr != dNode->owner)) ||
		   (nullptr == dNode->prev && (nullptr != dNode->next || nullptr != dNode->owner)) ||
		   (nullptr == dNode->owner && (nullptr != dNode->next || nullptr != dNode->prev)))
		{
			LogError() << "DoubLink Push error";
		}

		if(nullptr != dNode->next || nullptr != dNode->prev || nullptr != dNode->owner)
		{	
			return;
		}
	    tail_.prev->next = dNode;
	    dNode->prev      = tail_.prev;
	    tail_.prev        = dNode;
	    dNode->next      = &tail_;
	    dNode->owner     = this;
	    ++count_; 
	}

	void PushFront(DoubLinkNode *dNode)
	{
		if((nullptr == dNode->next && (nullptr != dNode->prev || nullptr != dNode->owner)) ||
		   (nullptr == dNode->prev && (nullptr != dNode->next || nullptr != dNode->owner)) ||
		   (nullptr == dNode->owner && (nullptr != dNode->next || nullptr != dNode->prev)))
		{
			LogError() << "DoubLink Push error";
		}

		if(nullptr != dNode->next || nullptr != dNode->prev || nullptr != dNode->owner)
		{	
			return;
		}
	    
		dNode->prev = &head_;
		dNode->next = head_.next;
		head_.next->prev = dNode;
		head_.next = dNode;		
		dNode->owner     = this;
	    ++count_; 		
	}


	DoubLinkNode *Pop()
	{
		if(head_.next == &tail_)
		{
			return nullptr;
		}
		DoubLinkNode *ret = head_.next;
		Remove(ret);
		return ret;
	}

	int32_t Empty(bool &isEmpty)
	{
		isEmpty = false;
		if(head_.next == &tail_)
		{
			isEmpty = true;
		}
		return 0;
	}

	static void Remove(DoubLinkNode *dNode)
	{

		if((nullptr != dNode->next && (nullptr == dNode->prev || nullptr == dNode->owner)) ||
		   (nullptr != dNode->prev && (nullptr == dNode->next || nullptr == dNode->owner)) ||
		   (nullptr != dNode->owner && (nullptr == dNode->next || nullptr == dNode->prev)))
		{
			LogError() << "DoubLink Remove error";
		}

		if(nullptr == dNode->prev || nullptr == dNode->next || nullptr == dNode->owner)
		{
			return;
		}
		
		dNode->prev->next = dNode->next;
		dNode->next->prev = dNode->prev;
		dNode->prev = dNode->next = nullptr;
		--dNode->owner->count_;
		dNode->owner = nullptr;
	}

private:
	DoubLink(const DoubLink &o);
	DoubLink& operator = (const DoubLink &o);

};

inline DoubLinkNode::~DoubLinkNode()
{
	if(nullptr != owner)
	{
		DoubLink::Remove(this);
	}
}


#endif
