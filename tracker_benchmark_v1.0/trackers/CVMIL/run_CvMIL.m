function results=run_CvMIL(seq, res_path, bSaveImage)

alg_type = 'MIL';

close all;

x=seq.init_rect(1)-1;%matlab to c
y=seq.init_rect(2)-1;
w=seq.init_rect(3);
h=seq.init_rect(4);

tic
tracker_dir = pwd;
cd ../../../'opencv_tracker/dev/bin/x64/Release';
tracker_program = '!opencv_tracker.exe';
command = [tracker_program...
    ' ' alg_type ' ' seq.name ' ' seq.path ' ' ...
    num2str(seq.startFrame) ' ' num2str(seq.endFrame) ' '  num2str(seq.nz) ' ' seq.ext ' '...
    num2str(x) ' ' num2str(y) ' ' num2str(w) ' ' num2str(h) ' '...
    num2str(bSaveImage) ' ' num2str(0) ' ' res_path ' ' res_path];
eval(command);
duration=toc;
cd(tracker_dir);
results.res = dlmread([res_path seq.name '_' alg_type '.txt']);   
results.res(:,1:2) =results.res(:,1:2) + 1;%c to matlab

results.type='rect';
results.fps=seq.len/duration;

results.fps = dlmread([res_path seq.name '_' alg_type '_FPS.txt']);


