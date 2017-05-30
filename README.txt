//Authors: Carter Reynolds and Alazar Genene 2/26/17
In tackling this virtual memory manager we created bit fields named present, valid, pfn, protection, and p inside the page table struct. 

The present bit indicates wherther or not the page in memory is being used at the moment, the valid bit is a way to show that the data has been allocated.

Protection makes sure you are able to write onto a page or not. A page has a pointer  to the next page and a physical address, the function to add a page makes in the adress the page is going to be mapped to as well as the page head(or the page to add's parent), and allocates pace for the new page and finally apppendds it to the the original by making it's next the newly created page.

The way we implemented removing pages was creating a new page setting it to Null and then setting the new page's adress to the heads and the new page to the the head's next and then freeing the head and returning the address.

Then we have a char *mem which simulates the memory buffer a global pg table field and a global swaps int to help in swapping to and from disk. 

Then there is a function called swap_to_disk that takes in a file a pointer to the head of the pages and an int evict, whicch will aid us in evicting pages. The swap counter is incremented at the start of the function call and a char of size 16 and a flag is set to one. 

Inside a for loop that goes until the 16 bytes of a page moves the memory to the initialized swap and then adds a page to the evicted page location in memory. If the frame to be evicted is the one we need to change where the page table points to using swap counter.

Then we need to find out if we are dealing with a page or a page table and if it's a page the page table in memory gets set to the required values same for a page table. 

The next funtion swaps a page from disk back into memory. The main function simulates memory and handles files being piped into the excecutable to be added to the page table in memory.
