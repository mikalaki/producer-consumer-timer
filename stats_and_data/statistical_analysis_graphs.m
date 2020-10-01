%% Statistical analysis of C timer program
%experiment 1: hourly experiment with 1 timer with period of 1 second
%experiment 2: hourly experiment with 1 timer with period of 0.1 seconds
%experiment 3: hourly experiment with 1 timer with period of 0.01 seconds
%experiment 4: hourly experiment with the 3 timers above alltogether.

clear all;

%finding the average execution time of a TimerFcn in Pi in microseconds.
pi_workFuncExecutionTimes= importdata('WorkFcnExecTimesPi.csv');
pi_average_workFuncExecutionTime= mean(pi_workFuncExecutionTimes);

%finding the average execution time of a TimerFcn in dev pc in microseconds.
dev_workFuncExecutionTimes= importdata('WorkFcnExecTimesDev.csv');
dev_average_workFuncExecutionTime= mean(dev_workFuncExecutionTimes);

%We use cell variables to store the data of every executions,as they differ in length 
producerPeriodDeclinations = cell(4,1); % e.g. producerPeriodDeclinations {1} is declination for T=1 seconds
actualPeriods = cell(4,1);
inQueueWaitingTimes= cell(4,1);

%We use cell arrays to store the data of every executions,as they differ in length 
FilteredProducerPeriodDeclinations = cell(4,1); % e.g. producerPeriodDeclinations {1} is declination for T=1 seconds
FilteredActualPeriods = cell(4,1);
FilteredInQueueWaitingTimes= cell(4,1);

%arrays to store mean values of data for the 4 experiments.
meanActualPeriods=zeros(4,1);
meanPeriodDeclinations=zeros(4,1);
meanInQueueWaitingTimes=zeros(4,1);

meanAbsActualPeriods=zeros(4,1);
meanAbsPeriodDeclinations=zeros(4,1);
meanAbsInQueueWaitingTimes=zeros(4,1);

%arrays to store min values of data for the 4 experiments.
minActualPeriods=zeros(4,1);
minPeriodDeclinations=zeros(4,1);
minInQueueWaitingTimes=zeros(4,1);

minAbsActualPeriods=zeros(4,1);
minAbsPeriodDeclinations=zeros(4,1);
minAbsInQueueWaitingTimes=zeros(4,1);

%arrays to store max values of data for the 4 experiments.
maxActualPeriods=zeros(4,1);
maxPeriodDeclinations=zeros(4,1);
maxInQueueWaitingTimes=zeros(4,1);

%arrays to store std (standart deviation) values of data for the 4 experiments.
stdActualPeriods=zeros(4,1);
stdPeriodDeclinations=zeros(4,1);
stdInQueueWaitingTimes=zeros(4,1);

%arrays to store median values of data for the 4 experiments.
medianActualPeriods=zeros(4,1);
medianPeriodDeclinations=zeros(4,1);
medianInQueueWaitingTimes=zeros(4,1);




%case 1,2,3 are for timer wiht period of 1, 0.1, 0.01 seconds respectively, and case 4 is for the 3 timers together 
for cases =[ 1 2 3 4 ]

      filename1=sprintf("producerAssignDelaysQUEUESIZE04_Q004_case%d.csv", cases);
      producerPeriodDeclinations{cases}=importdata(filename1);
        
      
      filename2=sprintf("InQueueWaitingTimesQUEUESIZE04_Q004_case%d.csv", cases);
      inQueueWaitingTimes{cases}=importdata(filename2);
      
      filename3=sprintf("actualPeriodsQUEUESIZE04_Q004_case%d.csv", cases);
      actualPeriods{cases}=importdata(filename3);
      
      
%       %Filter Data with moving average filter,       
%       FilteredProducerPeriodDeclinations{cases} = MA_filter(producerPeriodDeclinations{cases}, ...
%       floor(length(producerPeriodDeclinations{cases})/15));
%   
%       FilteredActualPeriods{cases} = MA_filter(actualPeriods{cases}, ...
%       floor(length(producerPeriodDeclinations{cases})/15));
        
      %Calculating Desired Statistics
      meanActualPeriods(cases)= mean(actualPeriods{cases});
      meanPeriodDeclinations(cases)= mean(producerPeriodDeclinations{cases});
      meanInQueueWaitingTimes(cases)= mean(inQueueWaitingTimes{cases});
      
      meanAbsActualPeriods(cases)= mean(abs(actualPeriods{cases}));
      meanAbsPeriodDeclinations(cases)= mean(abs(producerPeriodDeclinations{cases}));
      meanAbsInQueueWaitingTimes(cases)= mean(abs(inQueueWaitingTimes{cases}));
      
      maxActualPeriods(cases)= max(actualPeriods{cases});
      maxPeriodDeclinations(cases)= max(producerPeriodDeclinations{cases});
      maxInQueueWaitingTimes(cases)= max(inQueueWaitingTimes{cases});
      
      minActualPeriods(cases)= min(actualPeriods{cases});
      minPeriodDeclinations(cases)= min(producerPeriodDeclinations{cases});
      minInQueueWaitingTimes(cases)= min(inQueueWaitingTimes{cases});
      
      minAbsActualPeriods(cases)= min(abs(actualPeriods{cases}));
      minAbsPeriodDeclinations(cases)= min(abs(producerPeriodDeclinations{cases}));
      minAbsInQueueWaitingTimes(cases)= min(abs(inQueueWaitingTimes{cases}));
      
      stdActualPeriods(cases)= std(actualPeriods{cases});
      stdPeriodDeclinations(cases)= std(producerPeriodDeclinations{cases});
      stdInQueueWaitingTimes(cases)= std(inQueueWaitingTimes{cases});
      
      medianActualPeriods(cases)= median(actualPeriods{cases});
      medianPeriodDeclinations(cases)= median(producerPeriodDeclinations{cases});
      medianInQueueWaitingTimes(cases)= median(inQueueWaitingTimes{cases});
              
      
end

%% Graphs for period Declination
% %Period Declination plot for experiment 4.
figure();
bar(producerPeriodDeclinations{4},'BarWidth', 1);
ylim([-1.5e-4 1.5e-4])
hold on;
line([0,length(producerPeriodDeclinations{4})],[meanPeriodDeclinations(4),meanPeriodDeclinations(4)],'Color','red','LineWidth',1);
hold on;
line([0,length(producerPeriodDeclinations{4})],[meanPeriodDeclinations(4)-stdPeriodDeclinations(4), ...
    meanPeriodDeclinations(4)-stdPeriodDeclinations(4)],'Color','yellow','LineWidth',2);
hold on;
line([0,length(producerPeriodDeclinations{4})],[meanPeriodDeclinations(4)+stdPeriodDeclinations(4), ...
    meanPeriodDeclinations(4)+stdPeriodDeclinations(4)],'Color','yellow','LineWidth',2);
hold on;


title("Period Declinations (T=1 & T=0.1 & T=0.01 sec timers)");
xlabel("n. of Period");
ylabel("Period declination (seconds)");
legend("Period declinations","mean=6.9*10^-^1^0 sec", "mean-std=-9.7*10^-^5 s","mean+std=9.7*10^-^5 s");

hold off;

% %Period Declination plot for experiment 3.
figure();
bar(producerPeriodDeclinations{3},'BarWidth', 1);
ylim([-7e-5 7e-5])
hold on;
line([0,length(producerPeriodDeclinations{3})],[meanPeriodDeclinations(3),meanPeriodDeclinations(3)],'Color','red','LineWidth',1);
hold on;
line([0,length(producerPeriodDeclinations{3})],[meanPeriodDeclinations(3)-stdPeriodDeclinations(3), ...
    meanPeriodDeclinations(3)-stdPeriodDeclinations(3)],'Color','yellow','LineWidth',2);
hold on;
line([0,length(producerPeriodDeclinations{3})],[meanPeriodDeclinations(3)+stdPeriodDeclinations(3), ...
    meanPeriodDeclinations(3)+stdPeriodDeclinations(3)],'Color','yellow','LineWidth',2);
hold on;


title("Period Declinations (T=0.01 seconds timer)");
xlabel("n. of Period");
ylabel("Period declination (seconds)");
legend("Period declinations","mean=4.8*10^-^1^0 sec", "mean-std=-4.75*10^-^5 s","mean+std=4.75*10^-^5 s");

hold off;

% %Period Declination plot for experiment 2.
figure();
bar(producerPeriodDeclinations{2},'BarWidth', 1);
ylim([-3e-5 3e-5])
hold on;
line([0,length(producerPeriodDeclinations{2})],[meanPeriodDeclinations(2),meanPeriodDeclinations(2)],'Color','red','LineWidth',1);
hold on;
line([0,length(producerPeriodDeclinations{2})],[meanPeriodDeclinations(2)-stdPeriodDeclinations(2), ...
    meanPeriodDeclinations(2)-stdPeriodDeclinations(2)],'Color','yellow','LineWidth',2);
hold on;
line([0,length(producerPeriodDeclinations{2})],[meanPeriodDeclinations(2)+stdPeriodDeclinations(2), ...
    meanPeriodDeclinations(2)+stdPeriodDeclinations(2)],'Color','yellow','LineWidth',2);
hold on;


title("Period Declinations (T=0.1 seconds timer)");
xlabel("n. of Period");
ylabel("Period declination (seconds)");
legend("Period declinations","mean=3*10^-^9 sec", "mean-std=-1.16*10^-^5 s","mean+std=1.16*10^-^5 s");

hold off;


% %Period Declination plot for experiment 1.
figure();
bar(producerPeriodDeclinations{1},'BarWidth', 1);
ylim([-5e-5 5e-5])
hold on;
line([0,length(producerPeriodDeclinations{1})],[meanPeriodDeclinations(1),meanPeriodDeclinations(1)],'Color','red','LineWidth',1);
hold on;
line([0,length(producerPeriodDeclinations{1})],[meanPeriodDeclinations(1)-stdPeriodDeclinations(1), ...
    meanPeriodDeclinations(1)-stdPeriodDeclinations(1)],'Color','yellow','LineWidth',2);
hold on;
line([0,length(producerPeriodDeclinations{1})],[meanPeriodDeclinations(2)+stdPeriodDeclinations(1), ...
    meanPeriodDeclinations(1)+stdPeriodDeclinations(1)],'Color','yellow','LineWidth',2);
hold on;


title("Period Declinations (T=1 second timer)");
xlabel("n. of Period");
ylabel("Period declination (seconds)");
legend("Period declinations","mean=2.9*10^-^8 sec", "mean-std=-2.9*10^-^5 s","mean+std=2.9*10^-^5 s");

hold off;


%% Graphs for in-Queue Waiting times 
% %Waiting times plot for experiment 4.
figure();
bar(inQueueWaitingTimes{4},'BarWidth', 1);
ylim([0 70]);
hold on;
line([0,length(inQueueWaitingTimes{4})],[meanInQueueWaitingTimes(4),meanInQueueWaitingTimes(4)],'Color','red','LineWidth',1);
hold on;
line([0,length(inQueueWaitingTimes{4})],[meanInQueueWaitingTimes(4)-stdInQueueWaitingTimes(4), ...
    meanInQueueWaitingTimes(4)-stdInQueueWaitingTimes(4)],'Color','yellow','LineWidth',2);
hold on;
line([0,length(inQueueWaitingTimes{4})],[meanInQueueWaitingTimes(4)+stdInQueueWaitingTimes(4), ...
    meanInQueueWaitingTimes(4)+stdInQueueWaitingTimes(4)],'Color','yellow','LineWidth',2);
hold on;


title("Waiting times (T=1 & T=0.1 & T=0.01 sec timers)");
xlabel("n. of TimerFcn");
ylabel("Waiting time (microseconds)");
legend("Waiting times","mean=34.19 usec", "mean-std=27.18 usec","mean+std=41.2 usec");

hold off;

% %Waiting times plot for experiment 3.
figure();
bar(inQueueWaitingTimes{3},'BarWidth', 1);
ylim([0 70]);
hold on;
line([0,length(inQueueWaitingTimes{3})],[meanInQueueWaitingTimes(3),meanInQueueWaitingTimes(3)],'Color','red','LineWidth',1);
hold on;
line([0,length(inQueueWaitingTimes{3})],[meanInQueueWaitingTimes(3)-stdInQueueWaitingTimes(3), ...
    meanInQueueWaitingTimes(3)-stdInQueueWaitingTimes(3)],'Color','yellow','LineWidth',2);
hold on;
line([0,length(inQueueWaitingTimes{3})],[meanInQueueWaitingTimes(3)+stdInQueueWaitingTimes(3), ...
    meanInQueueWaitingTimes(3)+stdInQueueWaitingTimes(3)],'Color','yellow','LineWidth',2);
hold on;


title("Waiting times (T=0.01 seconds timer)");
xlabel("n. of TimerFcn");
ylabel("Waiting time (microseconds)");
legend("Waiting times","mean=30.59 usec", "mean-std=22.33 usec","mean+std=38.85 usec");

hold off;

% %Waiting time plot for experiment 2.
figure();
bar(inQueueWaitingTimes{2},'BarWidth', 1);
ylim([0 70]);
hold on;
line([0,length(inQueueWaitingTimes{2})],[meanInQueueWaitingTimes(2),meanInQueueWaitingTimes(2)],'Color','red','LineWidth',1);
hold on;
line([0,length(inQueueWaitingTimes{2})],[meanInQueueWaitingTimes(2)-stdInQueueWaitingTimes(2), ...
    meanInQueueWaitingTimes(2)-stdInQueueWaitingTimes(2)],'Color','yellow','LineWidth',2);
hold on;
line([0,length(inQueueWaitingTimes{2})],[meanInQueueWaitingTimes(2)+stdInQueueWaitingTimes(2), ...
    meanInQueueWaitingTimes(2)+stdInQueueWaitingTimes(2)],'Color','yellow','LineWidth',2);
hold on;


title("Waiting times (T=0.1 seconds timer)");
xlabel("n. of TimerFcn");
ylabel("Waiting time (microseconds)");
legend("Waiting times","mean=41.56 usec", "mean-std=32.35 usec","mean+std=50.77 usec");

hold off;


% %Waiting times plot for experiment 1.
figure();
bar(inQueueWaitingTimes{1},'BarWidth', 1);
ylim([0 70]);
hold on;
line([0,length(inQueueWaitingTimes{1})],[meanInQueueWaitingTimes(1),meanInQueueWaitingTimes(1)],'Color','red','LineWidth',1);
hold on;
line([0,length(inQueueWaitingTimes{1})],[meanInQueueWaitingTimes(1)-stdInQueueWaitingTimes(1), ...
    meanInQueueWaitingTimes(1)-stdInQueueWaitingTimes(1)],'Color','yellow','LineWidth',2);
hold on;
line([0,length(inQueueWaitingTimes{1})],[meanInQueueWaitingTimes(1)+stdInQueueWaitingTimes(1), ...
    meanInQueueWaitingTimes(1)+stdInQueueWaitingTimes(1)],'Color','yellow','LineWidth',2);
hold on;


title("Waiting times (T=1 second timer)");
xlabel("n. of TimerFcn");
ylabel("Waiting times (microseconds)");
legend("Waiting times","mean=32.97 usec", "mean-std=26.77 usec","mean+std=39.16 usec");

hold off;


% implementation of moving average filter
% output is the serie after the filter
% input is the input
function output= MA_filter(input, points)
    B = (1/points)*ones(points,1);
    output = filter(B,1,input);
end


