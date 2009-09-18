function [vA,vx_0,vy_0,vsigma_x,vsigma_y,va,vb,vc,vaA,vax_0,vay_0,vasigma_x,vasigma_y,vaa,vab,vac] = compare(massimo,minimo,centro_x,centro_y,var_x,var_y,cutx1,cutx2,cuty1,cuty2)
%COMPARE Function which ANALYZE pictures of a laser beam
%    Using both gaussian and airydisk fitting for every image

    iterazioni = 1000;
    
    CONVERSION_FACTOR = 200000;
    
    vA = zeros(1,iterazioni);
    vx_0 = zeros(1,iterazioni);
    vy_0 = zeros(1,iterazioni);
    vsigma_x = zeros(1,iterazioni);
    vsigma_y = zeros(1,iterazioni);
    va = zeros(1,iterazioni);
    vb = zeros(1,iterazioni);
    vc = zeros(1,iterazioni);
    
    vaA = zeros(1,iterazioni);
    vax_0 = zeros(1,iterazioni);
    vay_0 = zeros(1,iterazioni);
    vasigma_x = zeros(1,iterazioni);
    vasigma_y = zeros(1,iterazioni);
    vaa = zeros(1,iterazioni);
    vab = zeros(1,iterazioni);
    vac = zeros(1,iterazioni);
    
    %FIT INIZIALIZATION
    
    fp=fopen('results.dat','w');
    
    s = sprintf('/home/ligo/Desktop/newTest/Image/newtest1%03d.tiff',0);
    immagine = imread(s,'tif');
    image = immagine(cutx1:cutx2,cuty1:cuty2);
    figure(3);
    mesh(double(image));
    
    tic,[vA(1),vx_0(1),vy_0(1),vsigma_x(1),vsigma_y(1),va(1),vb(1),vc(1)] = gaussianFastFit(image,massimo,minimo,centro_x,centro_y,var_x,var_y),toc;
    
    fprintf(1,'INITIAL GAUSSIAN FIT COMPLETED\n');
        
    tic,[vaA(1),vax_0(1),vay_0(1),vasigma_x(1),vasigma_y(1),vaa(1),vab(1),vac(1)] = airyFastFit(image,massimo,minimo,centro_x,centro_y,var_x/CONVERSION_FACTOR,var_y/CONVERSION_FACTOR),toc;
    
    fprintf(fp,'%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n',vA(1),vx_0(1),vy_0(1),vsigma_x(1),vsigma_y(1),va(1),vb(1),vc(1),vaA(1),vax_0(1),vay_0(1),vasigma_x(1),vasigma_y(1),vaa(1),vab(1),vac(1));
    
    fprintf(1,'INITIAL AIRY FIT COMPLETED\n');
    
    for i=2:iterazioni
        i
        s = sprintf('/home/ligo/Desktop/newTest/Image/newtest1%03d.tiff',i-1);
        immagine = imread(s,'tif');
        image = immagine(cutx1:cutx2,cuty1:cuty2);
        
        [vA(i),vx_0(i),vy_0(i),vsigma_x(i),vsigma_y(i),va(i),vb(i),vc(i)] = oneShotGaussianFit(image,vA(i-1),vc(i-1),vx_0(i-1),vy_0(i-1),vsigma_x(i-1),vsigma_y(i-1),va(i-1),vb(i-1));
        
        [vaA(i),vax_0(i),vay_0(i),vasigma_x(i),vasigma_y(i),vaa(i),vab(i),vac(i)] = oneShotAiryFit(image,vaA(i-1),vac(i-1),vax_0(i-1),vay_0(i-1),vasigma_x(i-1),vasigma_y(i-1),vaa(i-1),vab(i-1));
        
        fprintf(fp,'%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n',vA(i),vx_0(i),vy_0(i),vsigma_x(i),vsigma_y(i),va(i),vb(i),vc(i),vaA(i),vax_0(i),vay_0(i),vasigma_x(i),vasigma_y(i),vaa(i),vab(i),vac(i));
    end
    
    x = linspace(1,iterazioni,iterazioni);
    
    figure(1);
    subplot(2,1,1), plot(x,vA);
    title('Amplitude deviation');
    xlabel('Time');
    ylabel('Amplitude');
    subplot(2,1,2), plot(x,vaA);
    xlabel('Time');
    ylabel('Amplitude');
    
    figure(2);
    subplot(2,1,1), plot(x,vx_0);
    title('X_0 deviation');
    xlabel('Time');
    ylabel('X_0');
    subplot(2,1,2), plot(x,vax_0);
    xlabel('Time');
    ylabel('X_0');
    
    figure(3);
    subplot(2,1,1), plot(x,vy_0);
    title('Y_0 deviation');
    xlabel('Time');
    ylabel('Y_0');
    subplot(2,1,2), plot(x,vay_0);
    xlabel('Time');
    ylabel('Y_0');
    
    figure(4);
    subplot(2,1,1), plot(x,vsigma_x);
    title('SIGMA_X deviation');
    xlabel('Time');
    ylabel('SIGMA_X');
    subplot(2,1,2), plot(x,CONVERSION_FACTOR.*vasigma_x);
    xlabel('Time');
    ylabel('SIGMA_X');
    
    figure(5);
    subplot(2,1,1), plot(x,vsigma_y);
    title('SIGMA_Y deviation');
    xlabel('Time');
    ylabel('SIGMA_Y');
    subplot(2,1,2), plot(x,CONVERSION_FACTOR.*vasigma_y);
    xlabel('Time');
    ylabel('SIGMA_Y');
    
    figure(6);
    subplot(2,1,1), plot(x,va);
    title('A deviation');
    xlabel('Time');
    ylabel('A');
    subplot(2,1,2), plot(x,vaa);
    xlabel('Time');
    ylabel('A');
    
    figure(7);
    subplot(2,1,1), plot(x,vb);
    title('B deviation');
    xlabel('Time');
    ylabel('B');
    subplot(2,1,2), plot(x,vab);
    xlabel('Time');
    ylabel('B');
    
    figure(8);
    subplot(2,1,1), plot(x,vc);
    title('C deviation');
    xlabel('Time');
    ylabel('C');
    subplot(2,1,2), plot(x,vac);
    xlabel('Time');
    ylabel('C');
    
    
end