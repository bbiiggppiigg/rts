/*
 * SCHED_SJF scheduling class. Implements a round robin scheduler with weight
 * priority mechanism.
 */

/*
 * Update the current task's runtime statistics. Skip current tasks that
 * are not in our scheduling class.
 */

 #define DEBUG 0

static void update_curr_sjf(struct rq *rq)
{
	struct task_struct *curr = rq->curr;
	u64 delta_exec;

	if (!task_has_sjf_policy(curr))
		return;

	delta_exec = rq->clock - curr->se.exec_start;
	if (unlikely((s64)delta_exec < 0))
		delta_exec = 0;

	schedstat_set(curr->se.exec_max, max(curr->se.exec_max, delta_exec));

	curr->se.sum_exec_runtime += delta_exec;
	curr->se.exec_start = rq->clock;
	cpuacct_charge(curr, delta_exec);
}

/*
 * Adding/removing a task to/from a priority array:
 */
static void enqueue_task_sjf(struct rq *rq, struct task_struct *p, int wakeup, bool b)
{
	// not yet implemented
	if(DEBUG==1) printk("ylc enqueue_task_sjf\n");
	struct sjf_rq  *sjf_rq = &(rq->sjf);
	list_add_tail(&(p->sjf_list_item), &(sjf_rq->queue));
	sjf_rq->nr_running++;
}

static void dequeue_task_sjf(struct rq *rq, struct task_struct *p, int sleep)
{
	// first update the task's runtime statistics
	if(DEBUG==1) printk("ylc-debug: dequeue_task_sjf\n");

	update_curr_sjf(rq);
	// not yet implemented
	struct sjf_rq  *sjf_rq = &(rq->sjf);
	list_del(&(p->sjf_list_item));
	sjf_rq->nr_running--;
}

/*
 * Put task to the end of the run list without the overhead of dequeue
 * followed by enqueue.
 */
static void requeue_task_sjf(struct rq *rq, struct task_struct *p)
{
	list_move_tail(&p->sjf_list_item, &rq->sjf.queue);
}

/*
 * current process is relinquishing control of the CPU
 */
static void
yield_task_sjf(struct rq *rq)
{
	// not yet implemented
	if(DEBUG==1) printk("ylc-debug: yield_task_sjf\n");
	struct task_struct *curr = rq->curr;
	requeue_task_sjf(rq, curr);
}

/*
 * Preempt the current task with a newly woken task if needed:
 * int wakeflags added to match function signature of other schedulers
 */
static void check_preempt_curr_sjf(struct rq *rq, struct task_struct *p, int wakeflags)
{
}

/*
 * select the next task to run
 */
static struct task_struct *pick_next_task_sjf(struct rq *rq)
{
	struct task_struct *next;
	struct list_head *queue;
	struct sjf_rq *sjf_rq;
	struct list_head * pos;
	// not yet implemented
	struct task_struct * tmp_next;
	if(DEBUG==1) printk("ylc-debug: pick_next_task_sjf\n");
	sjf_rq = &(rq->sjf);
	
	if(sjf_rq->nr_running>0){
		queue = &(sjf_rq->queue);
		next = list_first_entry(queue,struct task_struct,sjf_list_item);
		int min_value = next->sjf_time_slice;
		list_for_each(pos,queue){
			tmp_next = list_entry(pos,struct task_struct,sjf_list_item);
			if(tmp_next->sjf_time_slice < min_value){
				next = tmp_next;
				min_value = tmp_next->sjf_time_slice;
			}
		}
		return next;
	}
	

	// you need to return the selected task here 
	return NULL;
}

static void put_prev_task_sjf(struct rq *rq, struct task_struct *p)
{
	update_curr_sjf(rq);
	p->se.exec_start = 0;
}

#ifdef CONFIG_SMP
/*
 * Load-balancing iterator. Note: while the runqueue stays locked
 * during the whole iteration, the current task might be
 * dequeued so the iterator has to be dequeue-safe. Here we
 * achieve that by always pre-iterating before returning
 * the current task:
 */
static struct task_struct *load_balance_start_sjf(void *arg)
{	
	struct rq *rq = arg;
	struct list_head *head, *curr;
	struct task_struct *p;

	head = &rq->sjf.queue;
	curr = head->prev;

	p = list_entry(curr, struct task_struct, sjf_list_item);

	curr = curr->prev;

	rq->sjf.sjf_load_balance_head = head;
	rq->sjf.sjf_load_balance_curr = curr;

	return p;
}

static struct task_struct *load_balance_next_sjf(void *arg)
{
	struct rq *rq = arg;
	struct list_head *curr;
	struct task_struct *p;

	curr = rq->sjf.sjf_load_balance_curr;

	p = list_entry(curr, struct task_struct, sjf_list_item);
	curr = curr->prev;
	rq->sjf.sjf_load_balance_curr = curr;

	return p;
}

static unsigned long
load_balance_sjf(struct rq *this_rq, int this_cpu, struct rq *busiest,
		unsigned long max_load_move,
		struct sched_domain *sd, enum cpu_idle_type idle,
		int *all_pinned, int *this_best_prio)
{
	struct rq_iterator sjf_rq_iterator;

	sjf_rq_iterator.start = load_balance_start_sjf;
	sjf_rq_iterator.next = load_balance_next_sjf;
	/* pass 'busiest' rq argument into
	 * load_balance_[start|next]_sjf iterators
	 */
	sjf_rq_iterator.arg = busiest;

	return balance_tasks(this_rq, this_cpu, busiest, max_load_move, sd,
			     idle, all_pinned, this_best_prio, &sjf_rq_iterator);
}

static int
move_one_task_sjf(struct rq *this_rq, int this_cpu, struct rq *busiest,
		 struct sched_domain *sd, enum cpu_idle_type idle)
{
	struct rq_iterator sjf_rq_iterator;

	sjf_rq_iterator.start = load_balance_start_sjf;
	sjf_rq_iterator.next = load_balance_next_sjf;
	sjf_rq_iterator.arg = busiest;

	return iter_move_one_task(this_rq, this_cpu, busiest, sd, idle,
				  &sjf_rq_iterator);
}
#endif

/*
 * task_tick_sjf is invoked on each scheduler timer tick.
 */
static void task_tick_sjf(struct rq *rq, struct task_struct *p,int queued)
{
	struct task_struct *curr;
	struct sjf_rq *sjf_rq;
	// first update the task's runtime statistics
	struct list_head * pos, * queue;	
	update_curr_sjf(rq);
	// not yet implemented
	if(DEBUG==1) printk("ylc-debug: task_tick_sjf\n");
	sjf_rq = &(rq->sjf);
	curr = p;
	curr->task_time_slice--;
	int min_value = curr->sjf_time_slice;	
	int need_resched = 0;
/*
	if(sjf_rq->nr_running>0){
		 queue = &(sjf_rq->queue);
		list_for_each(pos,queue){
			if(list_entry(pos,struct task_struct,sjf_list_item)->sjf_time_slice < min_value){
				need_resched = 1;
				break;
			}
		}
	}
*/	
	if(curr->task_time_slice==0 /*|| need_resched*/){
		curr->task_time_slice=curr->sjf_time_slice;
		//requeue_task_sjf(rq, curr);
		//set_tsk_need_resched(p);
	}
	return;
}

/*
 * scheduling policy has changed -- update the current task's scheduling
 * statistics
 */
static void set_curr_task_sjf(struct rq *rq)
{
	struct task_struct *p = rq->curr;
	p->se.exec_start = rq->clock;
}

/*
 * We switched to the sched_sjf class.
 */
static void switched_to_sjf(struct rq *rq, struct task_struct *p,
			     int running)
{
	/*
	 * Kick off the schedule if running, otherwise just see
	 * if we can still preempt the current task.
	 */
	if (running)
		resched_task(rq->curr);
	else
		check_preempt_curr(rq, p, 0);
}

static int
select_task_rq_sjf(struct rq *rq, struct task_struct *p, int sd_flag, int flags)
{
	if (sd_flag != SD_BALANCE_WAKE)
		return smp_processor_id();

	return task_cpu(p);
}

const struct sched_class sjf_sched_class = {
	.next			= & idle_sched_class,
	.enqueue_task		= enqueue_task_sjf,
	.dequeue_task		= dequeue_task_sjf,
	.yield_task		= yield_task_sjf,

	.check_preempt_curr	= check_preempt_curr_sjf,

	.pick_next_task		= pick_next_task_sjf,
	.put_prev_task		= put_prev_task_sjf,

#ifdef CONFIG_SMP
	.load_balance		= load_balance_sjf,
	.move_one_task		= move_one_task_sjf,
#endif

	.switched_to  = switched_to_sjf,
	.select_task_rq = select_task_rq_sjf,

	.set_curr_task          = set_curr_task_sjf,
	.task_tick		= (void *)task_tick_sjf,
};
