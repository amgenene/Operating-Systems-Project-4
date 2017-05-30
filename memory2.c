//Authors: Carter Reynolds and Alazar Genene 2/26/17
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#define SIZE 64
#define PAGE_SIZE 16
struct page_table{
	u_int8_t present:1;
	u_int8_t valid:1;
	u_int8_t pfn:3;
	u_int8_t protection:1;
	u_int8_t pid:2;
};
struct page{
	int phyAddr;
	struct page * next;
};
void addPage(struct page **head,int phyAddr)
{
	struct page * new_page;
	new_page = malloc(sizeof(struct page));
	new_page-> next = *head;
	new_page->phyAddr = phyAddr;
	*head = new_page;
}
int removePage(struct page **head)
{
	struct page * new_page = NULL;
	if(*head==NULL)
		return -1;
	int addr = (*head)->phyAddr;
	new_page = (*head)->next;
	free(*head);
	*head = new_page;
	return addr;
}
const char *byte_to_binary(int x)
{
    static char b[9];
    b[0] = '\0';

    int z;
    for (z = 128; z > 0; z >>= 1)
    {
        strcat(b, ((x & z) == z) ? "1" : "0");
    }

    return b;
}
char memory[SIZE];
int pg_table_reg[4];
int swaps;
void swap_to_disk(FILE * fp,struct page ** head, int evict)
{
	swaps++;
	char swap[16];
	int flag=1;
    //for(int c=0;c<16;c++)
	//	printf("PreCopy %d:%d\n",c,memory[evict*16+c]);
	memmove(&swap, &memory[evict*16],16);
	addPage(head,evict*16);
	for(int j=0;j<4;j++)
	{
		//printf("PT %d at %d\n",j,pg_table_reg[j]);
		if(evict*16==pg_table_reg[j])
		{
			pg_table_reg[j]=((swaps-1)*-16)-2;
			flag = 0;
			//printf("Evict Table\n");
		}
	}
	//modify page table
	if(flag)
	{
		for(int i=0;i<4;i++)
		{
			for(int n=0;n<16;n++)
					//find page table entry
					if((memory[pg_table_reg[i]+n]&0b1111)==((evict<<2)|0b11))
					{
						//printf("Yo%s\n",byte_to_binary(memory[pg_table_reg[i]+n]));
						memory[pg_table_reg[i]+n]&=(0b11111110);
						memory[pg_table_reg[i]+n]&=(0b11100011);
						memory[pg_table_reg[i]+n]|=((swaps-1)<<2);
						//printf("Yo%s\n",byte_to_binary(memory[pg_table_reg[i]+n]));
	
					}
		}
	}
	printf("Swapped frame %d to disk at offset %d\n",evict,(swaps-1)*16);
	/*for(int k=0;k<16;k++)
	{
		if(swap[k]!=0)
			printf("Swap Values at %d:%s\n",k,byte_to_binary(swap[k]));	
	}*/
	fwrite(swap,1,sizeof(swap),fp);
	for(int i=0;i<PAGE_SIZE;i++)
	{
		memory[evict*16+i]=0;
	}
}
void swap_from_disk(FILE * fp,struct page ** head,int offset, int pid, int page,int v_Addr)
{
	char swap[16];
	int base=removePage(head);
	if(page)
	{
		offset = ((memory[pg_table_reg[pid]+v_Addr/16]&0b11100)>>2)*PAGE_SIZE;
	}
	else
		pg_table_reg[pid] = base;
	//printf("FILEEEEE DESCRIPTOR%d\n",(int)fp);
	//printf("%d==%d",(int)lseek((int)fp,(off_t)offset,SEEK_SET),offset);
	fseek(fp,(off_t)offset,SEEK_SET);
	fread(swap,1,sizeof(swap),fp);
	//fread is giving back different values than what was given to it
	/*for(int k=0;k<16;k++)
	{
		if(swap[k]!=0)
			printf("Swap Values at %d:%s\n",k,byte_to_binary(swap[k]));	
	}*/
	memmove(&memory[base],&swap,PAGE_SIZE);
	if(page)
	{
		memory[pg_table_reg[pid] + (v_Addr/16)]|=0b1;
		memory[pg_table_reg[pid] + (v_Addr/16)]&=(0b11100011);
		memory[pg_table_reg[pid] + (v_Addr/16)]|=((base/16)<<2);

	}
	printf("Swapped disk offset %d into frame %d\n",offset,base/PAGE_SIZE);
}

int main(void)
{
	for(int i=0;i<4;i++)
		pg_table_reg[i]=-1;
	for(int i=0;i<SIZE;i++)
		memory[i]=0;
	time_t t;
	srand(time(&t));	
	char * input=NULL;
	FILE *fp;
	fp=fopen("disk.txt","w+");
	//printf("File DESCRIPTOR:%d",(int)fp);
	size_t len = 0;
	int i=0;
	struct page * head = NULL;
	head = malloc(sizeof(struct page));
	head->phyAddr = 0;
	head->next = malloc(sizeof(struct page));
	head->next->phyAddr = 16;
	head->next->next = malloc(sizeof(struct page));
	head->next->next->phyAddr = 32;
	head->next->next->next = malloc(sizeof(struct page));
	head->next->next->next->phyAddr = 48;
	head->next->next->next->next = NULL;
	swaps=0;	
	for(int a=0;a<8;a++)
	{

		printf("Instruction? ");		
		while(getline(&input, &len, stdin)==-1);
		char * pid, * instruction, * vAddr,* value;
		//pid,instruct,virtual_address,value
		int p_id, v_Addr, v_alue;
		int evict = rand()%4;
		char *save=NULL,*save2;
		int flag=0;
		//read user input
		if((pid = strsep(&input,","))!=NULL)
		{
			sscanf(pid,"%d",&p_id);
			if((instruction = strsep(&input,","))!=NULL)
		    {
				if((vAddr = strsep(&input,","))!=NULL)
		    	{
		    		sscanf(vAddr,"%d",&v_Addr);
		    		if((value = strsep(&input,","))!=NULL)
				    	sscanf(value,"%d",&v_alue);
		    		else
		    			flag=1;
		    	}
		    	else
		    		flag=1;
		    }
		    else
		    	flag=1;
		}
		else
			flag=1;
		int base;
		//page table entry	
		int offset = pg_table_reg[p_id]+(v_Addr/16);
		//physical frame
		int pAddr=(memory[offset]&0b1100)>>2;
		//check user input
		if(flag)
			printf("Error:Instruction Parsing Failed\n");
		else if(p_id>3||p_id<0)
			printf("Error:Process ID Invalid\n");

		else if(strcmp(instruction,"map") == 0)
		{
			//if page table hasn't been created
			if(pg_table_reg[p_id]==-1)
			{
				//if there is a free page
				if((base=removePage(&head))!=-1)
				{
					int base1;
					printf("Put page table for PID %d into physical frame %d\n",p_id,base/16);
					pg_table_reg[p_id]=base;
					//if there is not a free page
					if((base1=removePage(&head))==-1)
					{
						swap_to_disk(fp,&head, evict);
						base1=removePage(&head);			
					}
					int page_num = base1/16;
					struct page_table new = {1,1,page_num,v_alue,p_id};
					//copy page table entry into page table
					memmove(&memory[base+v_Addr/16],&new,1);
					printf("\nMapped virtual address %d (page %d) into physical frame %d\n",v_Addr, v_Addr/16,page_num);
				}				
				else
				{
					swap_to_disk(fp,&head, evict);
					base=removePage(&head);
					int base1;
					printf("Put page table for PID %d into physical frame %d\n",p_id,base/16);
					pg_table_reg[p_id]=base;
					swap_to_disk(fp,&head, (evict+1)%4);
					base1=removePage(&head);
					int page_num=base1/16;
					struct page_table new={1,1,page_num,v_alue,p_id};
					memmove(&memory[base+v_Addr/16],&new,1);
					printf("\nMapped virtual address %d (page %d) into physical frame %d\n",v_Addr,v_Addr/16,page_num);					
				}
			}
			else if(pg_table_reg[p_id]>-1)
			{
				if(memory[offset]==0)
				{
					int base1;
					if((base1=removePage(&head))==-1)
					{
						swap_to_disk(fp,&head, evict);
						base1=removePage(&head);			
					}
					int page_num = base1/16;
					struct page_table new = {1,1,page_num,v_alue,p_id};
					memmove(&memory[base+v_Addr/16],&new,1);
					printf("\nMapped virtual address %d (page %d) into physical frame %d\n",v_Addr,v_Addr/16,page_num);
				}
				else
					printf("Page Table Entry Already Mapped\n");
			}
			else
			{
				if(memory[offset]==0)
				{
					swap_to_disk(fp,&head,evict);
					int offset = (pg_table_reg[p_id]+2)*-1;
					swap_from_disk(fp,&head,offset,p_id,0,v_Addr);
					swap_to_disk(fp,&head,(evict+1)%4);
					int base1=removePage(&head);			
					int page_num = base1/16;
					struct page_table new = {1,1,page_num,v_alue,p_id};
					memmove(&memory[base+v_Addr/16],&new,1);
					printf("\nMapped virtual address 0 (page 0) into physical frame %d\n",page_num);	
				}
				else
					printf("Page Table Entry Already Mapped\n");
			}
		}
		else if(strcmp(instruction,"store") == 0)
		{
				if(pg_table_reg[p_id]>-1)
				{
					int flag = 1;
					if((memory[offset]&0b100000)!=0b100000)
					{
						printf("Error: Memory is readable only");
						flag=0;
					}
					else if((memory[offset]&0b1)!=0b1)
					{	
						swap_to_disk(fp,&head,evict);
						swap_from_disk(fp,&head,offset,p_id,1,v_Addr);
						pAddr = (memory[offset]&0b1100)>>2;
					}
					if(flag)
					{
						memmove(&memory[v_Addr%16+(pAddr*16)],&v_alue,1);
						printf("Stored value %d at virtual address %d (physical address %d)\n",v_alue,v_Addr,pAddr*16+v_Addr%16);
					}
				}	
				else if(pg_table_reg[p_id]==-1)
					printf("Error: Page Table not mapped yet\n");
				else
				{
					swap_to_disk(fp,&head,evict);
					int offset = (pg_table_reg[p_id]+2)*-1;
					swap_from_disk(fp,&head,offset,p_id,0,v_Addr);
					if((memory[offset+v_Addr/16]&0b1)!=0b1)
					{	
						swap_to_disk(fp,&head,(evict+1)%4);
						swap_from_disk(fp,&head,offset,p_id,1,v_Addr);
						pAddr = (memory[offset]&0b1100)>>2;
					}
					memmove(&memory[v_Addr%16+pAddr*16],&v_alue,1);
					printf("Stored value %d at virtual address %d (physical address %d)\n",v_alue,v_Addr,pAddr*16+v_Addr%16);
				}
		
		}
		else if(strcmp(instruction,"load") == 0)
		{
			if(pg_table_reg[p_id] > -1)
			{
				//printf("%d offset:%d",memory[offset],offset);
				if((memory[offset]&0b1)!=0b1)
				{	
					swap_to_disk(fp,&head,evict);
					swap_from_disk(fp,&head,offset,p_id,1,v_Addr);
					pAddr = (memory[offset]&0b1100)>>2;
				}
				printf("The value %d is at virtual address %d (physical address %d)\n",memory[v_Addr+(pAddr*16)],v_Addr,pAddr*16+v_Addr);
			}
			else if(pg_table_reg[p_id] < -1)
			{
				swap_to_disk(fp,&head,evict);
				int offset = (pg_table_reg[p_id]+2)*-1;
				swap_from_disk(fp,&head,offset,p_id,0,v_Addr);
				printf("%d offset:%d",memory[offset],offset);
				if((memory[offset+v_Addr/16]&0b1)!=0b1)
				{	
					swap_to_disk(fp,&head,(evict+1)%4);
					swap_from_disk(fp,&head,offset,p_id,1,v_Addr);
					pAddr = (memory[offset]&0b1100)>>2;
				}
				printf("The value %d is at virtual address %d (physical address %d)\n",memory[v_Addr+(pAddr*16)],v_Addr,pAddr*16+v_Addr);
			}
			else
				printf("Error: Page Table not mapped yet\n");
		}
		else
		{
			printf("Error:Invalid Instruction\n");
		}
		/*for(int i=0;i<64;i++)
		{
			if(memory[i]!=0)
				printf("Phy Memory %d:%s\n",i,byte_to_binary(memory[i]));
		}*/
	}
	fclose(fp);   			
}
