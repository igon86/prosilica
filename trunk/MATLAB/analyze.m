function [vx_0,vy_0,vsigma_x,vsigma_y,va,vb,vc,vax_0,vay_0,vasigma_x,vasigma_y,vaa,vab,vac] = analyze(massimo,minimo,centro_x,centro_y,var_x,var_y,cutx1,cutx2,cuty1,cuty2)
%ANALYZE Function which ANALYZE pictures of a laser beam
%    Using both gaussian and airydisk fitting for every image

    iterazioni = 10;
    
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
    
    fp=fopen('results.dat','w');
    
    for i=1:iterazioni
        i
        s = sprintf('/home/ligo/Desktop/newTest/Image/newtest1%03d.tiff',i-1);
        immagine = imread(s,'tif');
        image = immagine(cutx1:cutx2,cuty1:cuty2);
        
        [vA(i),vx_0(i),vy_0(i),vsigma_x(i),vsigma_y(i),va(i),vb(i),vc(i)] = fastGaussianFit(image,massimo,minimo,centro_x,centro_y,var_x,var_y);
        
        [vaA(i),vax_0(i),vay_0(i),vasigma_x(i),vasigma_y(i),vaa(i),vab(i),vac(i)] = airyFastFit(image,massimo,minimo,centro_x,centro_y,0.0005,0.0005);
        
        fprintf(fp,'%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n',vA(i),vx_0(i),vy_0(i),vsigma_x(i),vsigma_y(i),va(i),vb(i),vc(i),vaA(i),vax_0(i),vay_0(i),vasigma_x(i),vasigma_y(i),vaa(i),vab(i),vac(i));
    end
    
%     for i=1:iterazioni
%         %s = sprintf('/home/ligo/Desktop/newTest/Image/newTest1%03d.tiff',i-1);
%         %immagine = imread(s,'tif');
%         image = immagine(cutx1:cutx2,cuty1:cuty2);
%         
%                 [vaA(i),vax_0(i),vay_0(i),vasigma_x(i),vasigma_y(i),vaa(i),vab(i),vac(i)] = airyFastFit(image,massimo,minimo,centro_x,centro_y,var_x,var_y);
%     end
    
    x = linspace(1,iterazioni,iterazioni);
    
    figure(1);
    subplot(2,1,1), plot(x,vx_0);
    subplot(2,1,2), plot(x,vax_0);
    figure(2);
    plot(x,vy_0);
    figure(3);
    plot(x,vsigma_x);
    figure(4);
    plot(x,vsigma_y);
    figure(5);
    plot(x,vax_0);
    figure(6);
    plot(x,vay_0);
    figure(7);
    plot(x,vsigma_x);
    figure(8);
    plot(x,vsigma_y);
    
end