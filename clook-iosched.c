/*
 * elevator clook
 *
 * Changes made ->
 *  All noop names were swapped to clook
 *  
 *  Dispatch was changed to record current disk location and 
 *  kprint when an io operation was dispatched
 *
 *  add_request was changed to implement C-look
 *
 */
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
//used to determine current position
int diskhead = -1;

struct clook_data {
	struct list_head queue;
};

static void clook_merged_requests(struct request_queue *q, struct request *rq,
				 struct request *next)
{
	list_del_init(&next->queuelist);
}

static int clook_dispatch(struct request_queue *q, int force)
{
	struct clook_data *nd = q->elevator->elevator_data;
	char io_type;

	if (!list_empty(&nd->queue)) {
		struct request *rq;
		rq = list_entry(nd->queue.next, struct request, queuelist);
		list_del_init(&rq->queuelist);
		elv_dispatch_sort(q, rq);
                // added value to keep track of current position and
		// added display for dispatch
		diskhead = blk_rq_pos(rq);
		if(rq_data_dir(rq) == READ){
		  io_type = 'R';
		}
		else{
		  io_type = 'W';
		}
		printk("[CLOOK] dsp %c %lu\n", io_type, blk_rq_pos(rq));




		
		return 1;
	}
	return 0;
}

static void clook_add_request(struct request_queue *q, struct request *rq)
{
	struct clook_data *nd = q->elevator->elevator_data;
	//pointer for location in linklist
	struct list_head *cur = NULL;
	//char for determing r or w
	char io_type;

	// List_for each will loop through the linked list
	// to find an appropaite location to insert
	//
	// This is the logic employed:
	//    if location > head:
	//         insert if the next is greater than request
	//         or if the linklist loops back to the start
	//    else
	//         insert if the next is greater ( and next is less than diskhead)
	//         or if we looped completly around
	list_for_each(cur, &nd->queue)
	  {
	    //struct to hold values of current location
	    struct request *location_request = list_entry(cur, struct request, queuelist);
	    // if request > blockhead
	    if(blk_rq_pos(rq) > diskhead)
	      {
		// if next > request
		if(blk_rq_pos(location_request) > diskhead &&
		   blk_rq_pos(rq) < blk_rq_pos(location_request)){
		  break;
		}
		// if next < blockhead ( looped back to beginning)
		else if(blk_rq_pos(location_request) < diskhead)
		  {
		    break;
		  }
	      }
	    //if request < blockhead
	    else{
	      // if next > request and next < blockhead
	      if(blk_rq_pos(location_request) < diskhead &&
		 blk_rq_pos(rq) < blk_rq_pos(location_request)){
		   break;
	      }
	      // if next == blockhead
	      else if (blk_rq_pos(rq) == diskhead)
		{
		  break;
		}
	    }
	  }
	//determine r or w and print add to log
	  if(rq_data_dir(rq) == READ){
	    io_type = 'R';
	  }
	  else{
	    io_type = 'W';
	  }
	    
	  printk("[CLOOK] add %c %lu\n", io_type, blk_rq_pos(rq));
	  //add to list where a spot was found.
	  list_add_tail(&rq->queuelist, cur);
}

static struct request *
clook_former_request(struct request_queue *q, struct request *rq)
{
	struct clook_data *nd = q->elevator->elevator_data;

	if (rq->queuelist.prev == &nd->queue)
		return NULL;
	return list_entry(rq->queuelist.prev, struct request, queuelist);
}

static struct request *
clook_latter_request(struct request_queue *q, struct request *rq)
{
	struct clook_data *nd = q->elevator->elevator_data;

	if (rq->queuelist.next == &nd->queue)
		return NULL;
	return list_entry(rq->queuelist.next, struct request, queuelist);
}

static int clook_init_queue(struct request_queue *q)
{
	struct clook_data *nd;

	nd = kmalloc_node(sizeof(*nd), GFP_KERNEL, q->node);
	if (!nd)
		return -ENOMEM;

	INIT_LIST_HEAD(&nd->queue);
	q->elevator->elevator_data = nd;
	return 0;
}

static void clook_exit_queue(struct elevator_queue *e)
{
	struct clook_data *nd = e->elevator_data;

	BUG_ON(!list_empty(&nd->queue));
	kfree(nd);
}

static struct elevator_type elevator_clook = {
	.ops = {
		.elevator_merge_req_fn		= clook_merged_requests,
		.elevator_dispatch_fn		= clook_dispatch,
		.elevator_add_req_fn		= clook_add_request,
		.elevator_former_req_fn		= clook_former_request,
		.elevator_latter_req_fn		= clook_latter_request,
		.elevator_init_fn		= clook_init_queue,
		.elevator_exit_fn		= clook_exit_queue,
	},
	.elevator_name = "clook",
	.elevator_owner = THIS_MODULE,
};

static int __init clook_init(void)
{
	return elv_register(&elevator_clook);
}

static void __exit clook_exit(void)
{
	elv_unregister(&elevator_clook);
}

module_init(clook_init);
module_exit(clook_exit);


MODULE_AUTHOR("Alexis Gonzalez");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("CLOOK IO scheduler");
