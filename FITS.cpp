#include <bits/stdc++.h>
using namespace std;

#define max_block 100
#define max_proc 100

void printblock(int blocksize[],int nblocks,int owner[]){
    cout<<"blocks\n";
    for (int i = 0; i < nblocks; i++){
        cout<<"Block"<<(i+1)<<"("<<blocksize[i]<<") :";
        if(owner[i] == 0) cout<<"free\n";
        else cout<<"P"<<owner[i]<<"\n";
    }
};

void bestfit(int nblocks,int blocksize[],int nproc,int procsize[]){
    cout<<"BEST fIT\n";
    int owner[max_block];
    for(int i=0; i<nblocks; i++) owner[i]=0;

    for(int p=0; p<nproc; p++){
        int need = procsize[p], pid=p+1;
        int bestindex = -1, bestwaste= 0;
        bool found = false;

        for(int b=0; b<nblocks; b++){
            if(owner[b]==0 && blocksize[b] >= need){
                int waste = blocksize[b]-need;
                if(!found || waste < bestwaste){
                    bestwaste = waste;
                    bestindex = b;
                    found = true;

                }
            }
        }
        if(bestindex != -1) owner[bestindex] = pid;
    }
    printblock( blocksize ,nblocks ,owner );
};

void firstfit(int nblocks,int blocksize[],int nproc,int procsize[]){
    cout<<"FIRST FIT \n";
    int owner[max_block];
    for(int i=0; i<nblocks; i++) owner[i]=0;

    for(int p=0; p<nproc; p++){
        int need = procsize[p], pid=p+1;
        for(int b=0; b<nblocks; b++){
            if(owner[b]==0 && blocksize[b] >= need) {owner[b]=pid; break;}
        }
    }
    printblock( blocksize ,nblocks ,owner );
};


void nextfit(int nblocks,int blocksize[],int nproc,int procsize[]){
    cout<<"NEXT fIT\n";
    int owner[max_block];
    for(int i=0; i<nblocks; i++) owner[i]=0;

    int pos =0;
    for(int p =0; p < nproc;p++){
        int need = procsize[p] , pid=p+1;
        bool placed = false;

        for(int k=0; k<nblocks; k++){
            int b = (pos+k) % nblocks;
            if(owner[b]==0 && blocksize[b] >= need ){
                owner[b] = pid;
                pos = b;
                break;
            }
        }

    }
    printblock( blocksize ,nblocks ,owner );
};


void worstfit(int nblocks,int blocksize[],int nproc,int procsize[]){
    cout<<"WORST fIT\n";
    int owner[max_block];
    for(int i=0; i<nblocks; i++) owner[i]=0;

    for (int p=0; p<nproc; p++){
        int need = procsize[p],pid=p+1;
        int worstindex = -1 , worstwaste = -1;

        for (int b=0; b<nblocks; b++){
            if(owner[b]==0 && blocksize[b] >= need){
                int waste = blocksize[b] - need;
            
                if(waste > worstwaste){
                    worstwaste=waste;
                    worstindex = b;
                }
            }
        }
        if(worstindex != -1 ) owner[worstindex]=pid;
    }
    printblock( blocksize ,nblocks ,owner );
}



int main(){
    int nblocks,nproc;
    int blocksize[max_block], procsize[max_proc];

    cin>>nblocks;
    for(int i=0; i<nblocks;i++){
        cin>>blocksize[i];
    }

    cin>>nproc;
    for(int i =0; i<nproc; i++){
        cin>>procsize[i];
    }

    bestfit(nblocks,blocksize,nproc,procsize);
    firstfit(nblocks,blocksize,nproc,procsize);
    nextfit(nblocks,blocksize,nproc,procsize);
    worstfit(nblocks,blocksize,nproc,procsize);


}
