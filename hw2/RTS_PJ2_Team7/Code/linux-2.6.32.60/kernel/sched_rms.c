/*
 * SCHED_rms scheduling class. Implements a round robin scheduler with weight
 * priority mechanism.
 */

/*
 * Update the current task's runtime statistics. Skip current tasks that
 * are not in our scheduling class.
 */

 #define DEBUG 0

static void update_curr_rms(struct rq *rq)
{
	struct task_struct *curr = rq->curr;
	u64 delta_exec;

	if (!task_has_rms_policy(curr))
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
static void enqueue_task_rms(struct rq *rq, struct task_struct *p, int wakeup, bool b)
{
	// not yet implemented
	if(DEBUG==1) printk("ylc enqueue_task_rms\n");
	struct rms_rq  *rms_rq = &(rq->rms);
	list_add_tail(&(p->rms_list_item), &(rms_rq->queue));
	rms_rq->nr_running++;
}

static void dequeue_task_rms(struct rq *rq, struct task_struct *p, int sleep)
{
	// first update the task's runtime statistics
	if(DEBUG==1) printk("ylc-debug: dequeue_task_rms\n");

	update_curr_rms(rq);
	// not yet implemented
	struct rms_rq  *rms_rq = &(rq->rms);
	list_del(&(p->rms_list_item));
	rms_rq->nr_running--;
}

/*
 * Put task to the end of the run list without the overhead of dequeue
 * followed by enqueue.
 */
static void requeue_task_rms(struct rq *rq, struct task_struct *p)
{
	list_move_tail(&p->rms_list_item, &rq->rms.queue);
}

/*
 * current process is relinquishing control of the CPU
 */
static void
yield_task_rms(struct rq *rq)
{
	// not yet implemented
	if(DEBUG==1) printk("ylc-debug: yield_task_rms\n");
	struct task_struct *curr = rq->curr;
	requeue_task_rms(rq, curr);
}

/*
 * Preempt the current task with a newly woken task if needed:
 * int wakeflags added to match function signature of other schedulers
 */
static void check_preempt_curr_rms(struct rq *rq, struct task_struct *p, int wakeflags)
{
}

/*
 * select the next task to run
 */
static struct task_struct *pick_next_task_rms(struct rq *rq)
{
	struct task_struct *next;
	struct list_head *queue;
	struct rms_rq *rms_rq;
	struct list_head * pos;
	// not yet implemented
	struct task_struct * tmp_next;
	if(DEBUG==1) printk("ylc-debug: pick_next_task_rms\n");
	rms_rq = &(rq->rms);
	
	if(rms_rq->nr_running>0){
		queue = &(rms_rq->queue);
		next = list_first_entry(queue,struct task_struct,rms_list_item);
		int min_value = next->rms_time_slice;
		list_for_each(pos,queue){
			tmp_next = list_entry(pos,struct task_struct,rms_list_item);
			if(tmp_next->rms_time_slice < min_value){
				next = tmp_next;
				min_value = tmp_next->rms_time_slice;
			}
		}
		return next;
	}
	

	// you need to return the selected task here 
	return NULL;
}

static void put_prev_task_rms(struct rq *rq, struct task_struct *p)
{
	update_curr_rms(rq);
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
static struct task_struct *load_balance_start_rms(void *arg)
{	
	struct rq *rq = arg;
	struct list_head *head, *curr;
	struct task_struct *p;

	head = &rq->rms.queue;
	curr = head->prev;

	p = list_entry(curr, struct task_struct, rms_list_item);

	curr = curr->prev;

	rq->rms.rms_load_balance_head = head;
	rq->rms.rms_load_balance_curr = curr;

	return p;
}

static struct task_struct *load_balance_next_rms(void *arg)
{
	struct rq *rq = arg;
	struct list_head *curr;
	struct task_struct *p;

	curr = rq->rms.rms_load_balance_curr;

	p = list_entry(curr, struct task_struct, rms_list_item);
	curr = curr->prev;
	rq->rms.rms_load_balance_curr = curr;

	return p;
}

static unsigned long
load_balance_rms(struct rq *this_rq, int this_cpu, struct rq *busiest,
		unsigned long max_load_move,
		struct sched_domain *sd, enum cpu_idle_type idle,
		int *all_pinned, int *this_best_prio)
{
	struct rq_iterator rms_rq_iterator;

	rms_rq_iterator.start = load_balance_start_rms;
	rms_rq_iterator.next = load_balance_next_rms;
	/* pass 'busiest' rq argument into
	 * load_balance_[start|next]_rms iterators
	 */
	rms_rq_iterator.arg = busiest;

	return balance_tasks(this_rq, this_cpu, busiest, max_load_move, sd,
			     idle, all_pinned, this_best_prio, &rms_rq_iterator);
}

static int
move_one_task_rms(struct rq *this_rq, int this_cpu, struct rq *busiest,
		 struct sched_domain *sd, enum cpu_idle_type idle)
{
	struct rq_iterator rms_rq_iterator;

	rms_rq_iterator.start = load_balance_start_rms;
	rms_rq_iterator.next = load_balance_next_rms;
	rms_rq_iterator.arg = busiest;

	return iter_move_one_task(this_rq, this_cpu, busiest, sd, idle,
				  &rms_rq_iterator);
}
#endif

/*
 * task_tick_rms is invoked on each scheduler timer tick.
 */
static void task_tick_rms(struct rq *rq, struct task_struct *p,int queued)
{
	struct task_struct *curr;
	struct rms_rq *rms_rq;
	// first update the task's runtime statistics
	update_curr_rms(rq);

	// not yet implemented
	if(DEBUG==1) printk("ylc-debug: task_tick_rms\n");
	rms_rq = &(rq->rms);
	curr = p;
	curr->task_time_slice--;
	if(curr->task_time_slice==0){
		curr->task_time_slice=curr->rms_time_slice;
		requeue_task_rms(rq, curr);
		set_tsk_need_resched(p);
	}
	return;
}

/*
 * scheduling policy has changed -- update the current task's scheduling
 * statistics
 */
static void set_curr_task_rms(struct rq *rq)
{
	struct task_struct *p = rq->curr;
	p->se.exec_start = rq->clock;
}

/*
 * We switched to the sched_rms class.
 */
static void switched_to_rms(struct rq *rq, struct task_struct *p,
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
select_task_rq_rms(struct rq *rq, struct task_struct *p, int sd_flag, int flags)
{
	if (sd_flag != SD_BALANCE_WAKE)
		return smp_processor_id();

	return task_cpu(p);
}

const struct sched_class rms_sched_class = {
	.next			= & idle_sched_class,
	.enqueue_task		= enqueue_task_rms,
	.dequeue_task		= dequeue_task_rms,
	.yield_task		= yield_task_rms,

	.check_preempt_curr	= check_preempt_curr_rms,

	.pick_next_task		= pick_next_task_rms,
	.put_prev_task		= put_prev_task_rms,

#ifdef CONFIG_SMP
	.load_balance		= load_balance_rms,
	.move_one_task		= move_one_task_rms,
#endif

	.switched_to  = switched_to_rms,
	.select_task_rq = select_task_rq_rms,

	.set_curr_task          = set_curr_task_rms,
	.task_tick		= (void *)task_tick_rms,
};
