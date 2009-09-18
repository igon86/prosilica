function [vA,vx_0,vy_0,vsigma_x,vsigma_y,va,vb,vc] = analyze(massimo,minimo,centro_x,centro_y,var_x,var_y,cutx1,cutx2,cuty1,cuty2)
%ANALYZE Function which ANALYZE pictures of a laser beam
%    Using a gaussian fitting algorithm for every image

    iterazioni = 1000;
    
    vA = zeros(1,iterazioni);
    vx_0 = zeros(1,iterazioni);
    vy_0 = zeros(1,iterazioni);
    vsigma_x = zeros(1,iterazioni);
    vsigma_y = zeros(1,iterazioni);
    va = zeros(1,iterazioni);
    vb = zeros(1,iterazioni);
    vc = zeros(1,iterazioni);
    
    %FIT INIZIALIZATION
    
    fp=fopen('results.dat','w');
    
    s = sprintf('/home/ligo/Desktop/CrashTest/CrashTest%03d.tiff',0);
    immagine = imread(s,'tif');
    image = immagine(cutx1:cutx2,cuty1:cuty2);
    
    tic,[vA(1),vx_0(1),vy_0(1),vsigma_x(1),vsigma_y(1),va(1),vb(1),vc(1)] = gaussianFastFit(image,massimo,minimo,centro_x,centro_y,var_x,var_y),toc;
    
    fprintf(1,'INITIAL GAUSSIAN FIT COMPLETED\n');
          
    fprintf(fp,'%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n',vA(1),vx_0(1),vy_0(1),vsigma_x(1),vsigma_y(1),va(1),vb(1),vc(1));
    
    
    for i=2:iterazioni
        i
        s = sprintf('/home/ligo/Desktop/newTest/Image/newtest1%03d.tiff',i-1);
        immagine = imread(s,'tif');
        image = immagine(cutx1:cutx2,cuty1:cuty2);
        
        [vA(i),vx_0(i),vy_0(i),vsigma_x(i),vsigma_y(i),va(i),vb(i),vc(i)] = oneShotGaussianFit(image,vA(i-1),vc(i-1),vx_0(i-1),vy_0(i-1),vsigma_x(i-1),vsigma_y(i-1),va(i-1),vb(i-1));
        
        
        fprintf(fp,'%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n',vA(i),vx_0(i),vy_0(i),vsigma_x(i),vsigma_y(i),va(i),vb(i),vc(i));
    end
    
    x = linspace(1,iterazioni,iterazioni);
    
    figure(1);
    plot(x,vA);
    title('Amplitude deviation');
    xlabel('Time');
    ylabel('Amplitude');
    
    figure(2);
    plot(x,vx_0);
    title('X_0 deviation');
    xlabel('Time');
    ylabel('X_0');
    
    figure(3);
    plot(x,vy_0);
    title('Y_0 deviation');
    xlabel('Time');
    ylabel('Y_0');
    
    figure(4);
    plot(x,vsigma_x);
    title('SIGMA_X deviation');
    xlabel('Time');
    ylabel('SIGMA_X');
    
    figure(5);
    plot(x,vsigma_y);
    title('SIGMA_Y deviation');
    xlabel('Time');
    ylabel('SIGMA_Y');
    
    figure(6);
    plot(x,va);
    title('A deviation');
    xlabel('Time');
    ylabel('A');
    
    figure(7);
    plot(x,vb);
    title('B deviation');
    xlabel('Time');
    ylabel('B');
    
    figure(8);
    plot(x,vc);
    title('C deviation');
    xlabel('Time');
    ylabel('C');
    
    
end