%plot fps

data = load('/cvs/cds/caltech/target/Prosilica/40mCode/SnapCode/SnapPyClean/frameRateTimestamp.txt');

for x = 1:(length(data)-1)
diff(x) = data(x+1,1) - data(x,1);
end

fps = 1000./diff

figure(1)
scatter((data([1:length(data)-1],1)-data(1,1))/60,fps)
xlabel('Time (minutes)')
ylabel('Frames per second')
title('FPS vs duration of code running constantly - 640x480 with 0.01 s exposure')

figure(2)
hist(fps,25)
xlabel('Frames per second')
ylabel('Number')
title('Histogram of FPS - 640x480 with 0.01 s exposure')