#ifndef _LINKED_LIST_H_
#define _LINKED_LIST_H_

#define LIST_SIZE 10000 
#define INIT_VAL -1

typedef struct mem_block
{
	struct mem_block *next;//@ Pointer to next block 
	int value;            //@ Pointer to the value stored in the block

}block;
	
typedef struct linked_list
{
	int cnt;       //@ Number of blocks in the list
	block *head;   //@ Address of first block's structure

}linked_list;

linked_list list;//@ Instantization of the linked list register

/******************************************* 
 @ Creates a linked list of size LIST_SIZE @
*******************************************/
void dummy_create(void)
{
	block *firstblock = NULL;
	
	/*@ Create and initialize the head of the linked list @*/	
	firstblock = (block *)kmalloc(sizeof(block),GFP_KERNEL);
	firstblock->next = NULL;
	firstblock->value = INIT_VAL;	
	
	/*@ Update the head address in the list structure @*/
	list.head = firstblock;
	
	/*@ Add blocks to the list @*/
	for(list.cnt=1; list.cnt<LIST_SIZE; list.cnt++){
		
		block *newblock = list.head;//@ Direct the pointer to the head
		
		if(newblock != NULL){//@ If that memory is properly allocated
			while(newblock->next != NULL)//@ Look for the list's tail
				newblock = newblock->next;
		}
		/*@ Allocate memory for the new tail block @*/	
		newblock->next = (block *)kmalloc(sizeof(block),GFP_KERNEL);
		newblock = newblock->next;//@ Redirect the pointer to the new allocation
		
		if(newblock == 0){
			printk(KERN_ERR "kmalloc error, out of memory\n");
			return;
		}
		/*@ Initialize the data of the new block  @*/
		newblock->next = NULL;
		newblock->value = INIT_VAL;
	}
	printk("@ Linked list created (list.cnt = %d)\n",list.cnt);	
	
}
/******************************************** 
 @ Free memory allocated by the linked list @
********************************************/
void dummy_destroy(void)
{
	block *dummyblock = list.head;//@ Point at the head
	block *poorblock = NULL;
	
	while(list.cnt > 0){
		
		poorblock = dummyblock;//@ Save the address of the block to delete
		dummyblock = dummyblock->next;//@ Point to the next block in the list
		kfree(poorblock);//@ Free the poor block
		list.cnt--;
	}
	printk("@ Linked list destroyed (list.cnt = %d)\n",list.cnt);		
}

#endif //_LINKED_LIST_H_