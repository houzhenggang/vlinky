#include "queue.h"

int initialize_queue(void *buffer,int size,int queue_ele_nr)
{
	struct queue_stub * stub;
	if(size<sizeof(struct queue_stub))
		return -1;
	stub=(struct queue_stub*)buffer;
	stub->front_ptr=0;
	stub->rear_ptr=0;
	stub->ele_num=queue_ele_nr;
	stub->ele_size=sizeof(struct queue_element);
	if((size-sizeof(struct queue_stub))<((stub->ele_num+1)*stub->ele_size))
		return -1;
	return 0;
}


static int queue_full(struct queue_stub *stub)
{
	return ((stub->rear_ptr+1)%(stub->ele_num+1))==stub->front_ptr;
}
static int queue_empty(struct queue_stub *stub)
{
	return stub->rear_ptr==stub->front_ptr;
}

int queue_quantum(struct queue_stub *stub)
{
	return stub->ele_num-((stub->rear_ptr-stub->front_ptr+stub->ele_num+1)%(stub->ele_num+1));
}
/*negative return value indicates a failure*/
int enqueue_single(struct queue_stub* stub,struct queue_element *ele)
{
	if(queue_full(stub))
		return -1;
	stub->records[stub->rear_ptr].rte_pkt_offset=ele->rte_pkt_offset;
	stub->records[stub->rear_ptr].rte_data_offset=ele->rte_data_offset;
	WRITE_MEM_WB();/*this is essential because here we can not guanrantee order of the write operation issues*/
	stub->rear_ptr=(stub->rear_ptr+1)%(stub->ele_num+1);
	return 0;
}
int dequeue_single(struct queue_stub *stub,struct queue_element*ele)
{
	if(queue_empty(stub))
		return -1;
	ele->rte_pkt_offset=stub->records[stub->front_ptr].rte_pkt_offset;
	ele->rte_data_offset=stub->records[stub->front_ptr].rte_data_offset;
	WRITE_MEM_WB();
	stub->front_ptr=(stub->front_ptr+1)%(stub->ele_num+1);
	return 0;
}
/*return value is the number oflelment that has been commited*/
int enqueue_bulk(struct queue_stub *stub,struct queue_element **ele_arr,int length)
{
	int idx=0;
	int rc;
	for(idx=0;idx<length;idx++){
		rc=enqueue_single(stub,ele_arr[idx]);
		if(rc<0)
			break;
	}
	return idx;
}

int dequeue_bulk(struct queue_stub * stub,struct queue_element **ele_arr,int max_length)
{	
	int idx=0;
	int rc=0;
	for(idx=0;idx<max_length;idx++){
		rc=dequeue_single(stub,ele_arr[idx]);
		if(rc<0)
			break;
	}
	return idx;
}