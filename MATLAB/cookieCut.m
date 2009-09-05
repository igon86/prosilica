function [mask,Ampiezza,minimo,x_0,y_0,sigma_x,sigma_y] = cookieCut(image,alpha)
    
    %dimension of the image
    [dimx,dimy] = size(image);
    img = image(:);
    m = size(img);
    
    %max e min luminosity of the image
    massimo = max(img);
    minimo = min(img);
    
    Ampiezza = massimo;
    
    %luminosity threshold
    threshold = massimo*alpha;
    
    %inizialization of the mask & count structures
    mask = zeros(m,1);
    countw=zeros(dimx,1);
    counth=zeros(dimy,1);
    count=0;
    
    %mask filling
    for i = 1:m
        if img(i) > threshold
            mask(i) = 1;
            count=count+1;
            countw(mod(i,dimx)+1) = countw(mod(i,dimx)+1)+1;
            counth(floor(i/dimy)+1) = counth(floor(i/dimy)+1)+1;
            
        end
    end
%     figure(2);
%     plot(countw);
%     figure(3);
%     plot(counth);
    %centre detection
    wcenter = 0;
    hcenter = 0;
    for i=1:dimx
        wcenter = wcenter + (countw(i)/count)*i;
    end
    for i=1:dimx
        hcenter = hcenter + (counth(i)/count)*i;
    end
    
    x_0 = wcenter;
    y_0 = hcenter;
    
    %approximation -> can be removed
    wcenter = round(wcenter);
    hcenter = round(hcenter);
    
    %sigma detection
    
    FWHM =massimo/2
    tempx1 = wcenter;
    tempx2 = wcenter;
    tempy1 = hcenter;
    tempy2 = hcenter;
    
    %sigma_x detection
    while(image(hcenter,tempx1)>FWHM)
        
        tempx1 = tempx1+1
    end
    
    while(image(hcenter,tempx2)>FWHM)
        tempx2 = tempx2-1
    end
    
    FWHM_x = 0.5*(tempx1-wcenter) + 0.5*abs(tempx2-wcenter);
    %we know that FWHM -= 2,35sigma
    sigma_x = 0.85*FWHM_x;
    
    
    
    %sigma_y detection
    while(image(tempy1,wcenter) > FWHM)
        tempy1 = tempy1+1;
    end
    
    while(image(tempy2,wcenter) > FWHM)
        tempy2 = tempy2-1;
    end
    
    FWHM_y = 0.5*(tempy1-hcenter) + 0.5*abs(tempy2-hcenter);
    sigma_y = 0.85*FWHM_y;
    
    fprintf(1,'Centro in %d - %d con ampiezza %d e deviazioni sui due assi %d - %d\n',round(wcenter),round(hcenter),Ampiezza,round(sigma_x),round(sigma_y));
    mask = reshape(mask,dimx,dimy);
end