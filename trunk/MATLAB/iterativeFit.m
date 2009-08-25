function [] = iterativeFit(image)
    
    %cookie cutting with prediction of initial parameters. 
    %0.6 is a number that usually gives us good results for
    %approssimation...
    [mask,A,minimo,x_0,y_0,sigma_x,sigma_y] = cookieCut(image,0.6);
    
    %plot della roba da fittare
    figure(1);
    imag = double(image);
    mesh(imag);
    title('toBeFitted');
    
    figure(2);
    imagesc(mask);
    title('cookieCut');
    
    fprintf(1,'tentiamo il fit con maschera');
    
    [Amplitude,x0,y0,var_x,var_y,a,b,c] = gaussianFit(image,mask,194,minimo,x_0,y_0,sigma_x,sigma_y);
end