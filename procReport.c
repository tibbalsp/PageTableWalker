#include<linux/module.h>
#include<linux/sched/signal.h>
#include<linux/pid_namespace.h>
#include<asm/io.h>

// Author: Patrick Tibbals



//Virtual to Physical
int vert2phys(struct mm_struct *input,unsigned long vpage){
	

	pgd_t *pgd;// Page directory, 5th level
	p4d_t *p4d;// Page directory, 4th level
	pud_t *pud;// Page upper directory, 3rd level
	pmd_t *pmd;// Page middle directory, 2nd level	
	pte_t *pte;// Page table entry, 1st level	
	
	
	unsigned long physical_page_addr = 0;
	struct page *page;

//pgd_offset -> Returns pointer to page directory entry of an
//		address given a pointer to the mm_struct
	pgd = pgd_offset(input, vpage); 
	if (pgd_none(*pgd) || pgd_bad(*pgd))
		return 0;
		
//p4d_offset -> Returns pointer to P4D entry of an
//		address given a pointer	to the mm_struct	
	p4d = p4d_offset(pgd, vpage);
	if (p4d_none(*p4d) || p4d_bad(*p4d)) 
		return 0;
		
//pud_offset -> Returns pointer to PUD entry of an
//		address given a pointer to the PGD entry		
	pud = pud_offset(p4d, vpage);
	if (pud_none(*pud) || pud_bad(*pud))
		return 0; 
		
//pmd_offset -> Returns pointer to the PMD entry of an
//		address given a pointer to the PUD entry		
	pmd = pmd_offset(pud, vpage);
	if (pmd_none(*pmd) || pmd_bad(*pmd)) 
		return 0;


// pte == Page Table Entry		
// pte_offset_map -> Yields and address of the entry in the page table 
//		     that corrisponds to the provided PMD entry.
//		     Establishes a temporary kernal mapping which is released using pte_unmap()	
	if (!(pte = pte_offset_map(pmd, vpage))) 
		return 0;
		
// pte_page -> Returns a pointer to the struct page entry 
//	       that corrisponds to the provided PTE(Page table entry)
	if (!(page=pte_page (*pte)))
		return 0; 

// 	Get the address		S
	physical_page_addr = page_to_phys(page);
//	Release the map
	pte_unmap(pte);
	
//	Handle unmapped page
	if (physical_page_addr==70368744173568)
		return 0; 
		
//	Return if page exsists 	
	return physical_page_addr;
}

int proc_count(void){

	int i=0;
	int process_page_total = 0;
	int all_page_total = 0;
	struct task_struct *thechild;
	printk(KERN_INFO "PROCESS REPORT:\n");
	printk(KERN_INFO "PROC_ID , PROC_NAME , TOTAL PAGES\n");
	for_each_process(thechild){
		
		struct vm_area_struct *vma;
		unsigned long vpage;
		unsigned long physical_page_addr;
		
		if(thechild->mm && thechild->mm->mmap){
			// Each block of virtual memory 
			for(vma = thechild->mm->mmap; vma; vma =  vma->vm_next){
				// Each page in the block of virtual memory 
				for(vpage = vma->vm_start; vpage< vma->vm_end; vpage += PAGE_SIZE){
					
					physical_page_addr = vert2phys(thechild->mm, vpage);
					// If address is 0 the page is not allocated
					if(physical_page_addr != 0){
						
						if(thechild->pid > 650){
						 
						 	process_page_total++;
						}
						 
						all_page_total++;
					}
				}
			}						
				
		}
		
		// Only print out those with PID > 650
	 	if(thechild->pid > 650){
			printk(KERN_INFO "%d , %s , %d",thechild->pid , thechild->comm , process_page_total);
			process_page_total = 0;
		
		}
		
		i++;

	}
	
	return all_page_total;

}
int proc_init (void) {

	printk(KERN_INFO "procReport: kernel module initialized\n");
	printk(KERN_INFO "TOTAL PAGES: %d",proc_count());
	
	return 0;
}



void proc_cleanup(void) {

  printk(KERN_INFO "procReport: performing cleanup of module\n");

}

MODULE_LICENSE("GPL");
module_init(proc_init);
module_exit(proc_cleanup);


