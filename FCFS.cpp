#include <bits/stdc++.h> 
using namespace std;


int main(){

    cout<<"enter n of proc: ";
    int n;
    cin>>n;

    int pid[20],at[20],bt[20],ct[20],tat[20],wt[20],rbt[20];

    for(int i=0;i<n;i++){
        pid[i]=i+1;
        cout<<"arrival time of P "<<pid[i]<<endl;
        cin>>at[i];
        cout<<"burst time of P "<<pid[i]<<endl;
        cin>>bt[i];
        rbt[i]=bt[i];
    }

    //FCFS 

    cout<<"FCSC \n";
    int time =0;

    for (int i=0;i<n;i++){
        if (time<at[i]) time = at[i];
        time += bt[i];
        ct[i]=time;
        tat[i]=ct[i]-at[i];
        wt[i]=tat[i]-bt[i];
    }

    cout<<"Pr  at  bt  wt  tat"<<endl;
    for (int i=0;i<n;i++){
        cout<<pid[i]<<"   "<<at[i]<<"   "<<bt[i]<<"   "<<wt[i]<<"   "<<tat[i]<<"\n";
    }

    cout<<"ROUND ROBIN \n";
    cout<<"enter quantum num : ";
    int q;
    cin>>q;
    int completed = 0;;
    time =0;

    for(int i=0;i<n;i++){
        rbt[i] = bt[i];
        wt[i] = ct[i] = tat[i] =0;
    }

    while(completed <n){
        bool work = false;
        for (int i=0;i<n;i++){
            if(at[i] <= time && rbt[i] >0){
                work = true;

                if(rbt[i] > q){
                    time+=q;
                    rbt[i]-=q;
                }
                else{
                    time += rbt[i];
                    rbt[i]=0;

                    ct[i]=time;
                    tat[i]=ct[i]-at[i];
                    wt[i]=tat[i]-bt[i];
                    completed ++;
                }
            }
        }
        if(!work) time++;
    }

    cout<<"Pr  at  bt  wt  tat"<<endl;
    for (int i=0;i<n;i++){
        cout<<pid[i]<<"   "<<at[i]<<"   "<<bt[i]<<"   "<<wt[i]<<"   "<<tat[i]<<"\n";
    }
}
