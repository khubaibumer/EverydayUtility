#ifndef __CONTAINER_H__
#define __CONTAINER_H__

#ifndef EXPORT_SYMBOL
 #define EXPORT_SYMBOL(x)
#endif	/*	EXPORT_SYMBOL	*/

#ifndef __KERNEL__	///< Make Compatible with LKM
 #define kmalloc(x, y) calloc(1, x)
 #define kfree(x) free(x)
#endif	/*	__KERNEL__	*/

/*		Generic Link-list to store credentials	*/
typedef struct data_node {
	void *data;
	struct data_node *prev, *next;
} data_node_t;

/*	Check if node is empty	*/
#define is_empty_node(head)												\
	({																	\
		(data_node_t*) (*head) == NULL ? 1 : 0;							\
	})
EXPORT_SYMBOL(is_empty_node);

/*	Find Data in list		*/
#define find_node(head, _data, len)										\
	({																	\
		data_node_t *node = (data_node_t*) *head;						\
		while(node != NULL) {											\
			if(memcmp(_data, node->data, len) == 0)						\
				break;													\
			else  														\
				node = node->next;										\
		}																\
		(node);															\
	})
EXPORT_SYMBOL(find_node);

/*		Insert Data at the top of stack		*/
#define insert_node(head, _data)										\
	({																	\
		data_node_t *node = kmalloc(sizeof(data_node_t), GFP_KERNEL);	\
		node->data = _data;												\
		node->prev = NULL;												\
		node->next = (data_node_t*) (*head);							\
		if(((data_node_t**) *head) != NULL)								\
			(*head)->prev = node;										\
		*head = node;													\
		(*head);														\
	})
EXPORT_SYMBOL(insert_node);

/*		Delete Data top of stack		*/
#define delete_node(_head, _node)												\
	({																			\
		do{																		\
			data_node_t **head = _head;											\
			data_node_t *node = _node;											\
			if(*(head) == NULL	|| (node) == NULL)								\
				break;															\
			if(*(head) == (node))												\
				*(head) = (node->next);											\
			if((node->next) != NULL)											\
				(node->next->prev) = (node->prev);								\
			if((node->prev) != NULL)											\
				(node->prev->next) = (node->next);								\
			kfree(node);														\
		} while(0);																\
	})
EXPORT_SYMBOL(delete_node)

/*		Delete a complete List		*/
#define delete_list_(_head)														\
	({																			\
		data_node_t *_head_ = (data_node_t*) *_head;							\
		do{																		\
			if(is_empty_node(&_head_))											\
				break;															\
			;;;																	\
			while(!is_empty_node(&_head_)) {									\
				delete_node(&_head_, _head_);									\
			}																	\
		} while(0);																\
	})
EXPORT_SYMBOL(delete_list);

/*		foreach_node_callback register		*/
/*		void (*callback_f)(data_node_t*)	*/
#define foreach_node_callback(_head, FUNC)										\
	({																			\
		data_node_t *node = (data_node_t*) *_head;								\
		do{																		\
			if(is_empty_node(&node))											\
				break;															\
			;;;																	\
			while(node != NULL)	{												\
				FUNC(node);														\
				node = node->next;												\
			}																	\
		} while(0);																\
	})
EXPORT_SYMBOL(foreach_node_callback)

#endif /*	__CONTAINER_H__	*/
