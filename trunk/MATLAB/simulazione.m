function [x,y,xa,ya] = simulazione(massimo,minimo,centro_x,centro_y,var_x,var_y,cutx1,cutx2,cuty1,cuty2)

    iterazioni = 100;
    numpixel = 40000;
    r = (2^0.5)*numpixel;
    
    %[vA,vx_0,vy_0,vsigma_x,vsigma_y,va,vb,vc] =zeros(iterazioni,1);
    
    for i=1:iterazioni
        s = sprintf('/home/ligo/Desktop/Image/strean%03d.tiff',i-1);
        immagine = imread(s,'tif');
        image = immagine(cutx1:cutx2,cuty1:cuty2);
        
        %[vA(i),vx_0(i),vy_0(i),vsigma_x(i),vsigma_y(i),va(i),vb(i),vc(i)] = fastGaussianFit(image,massimo,minimo,centro_x,centro_y,var_x,var_y);
        quad = immagine(1:400,150:550);
        quad1 = quad(200:400,1:200);
        quad2 = quad(200:400,200:400);
        quad3 = quad(1:200,1:200);
        quad4 = quad(1:200,200:400);
        sum1(i) = sum(sum(quad1));
        sum2(i) = sum(sum(quad2));
        sum3(i) = sum(sum(quad3));
        sum4(i) = sum(sum(quad4));
        x(i) = (-sum1(i)/r)+(-sum3(i)/r)+(sum2(i)/r)+(sum4(i)/r);
        y(i) = (sum1(i)/r)+(sum2(i)/r)+(-sum3(i)/r)+(-sum4(i)/r);
    end
    
    for i=1:iterazioni
        s = sprintf('/home/ligo/Desktop/Image/agitata%03d.tiff',i-1);
        immagine = imread(s,'tif');
        image = immagine(cutx1:cutx2,cuty1:cuty2);
        
        %[vaA(i),vax_0(i),vay_0(i),vasigma_x(i),vasigma_y(i),vaa(i),vab(i),vac(i)] = fastGaussianFit(image,massimo,minimo,centro_x,centro_y,var_x,var_y);
        quad = immagine(1:400,150:550);
        quad1 = quad(200:400,1:200);
        quad2 = quad(200:400,200:400);
        quad3 = quad(1:200,1:200);
        quad4 = quad(1:200,200:400);
        suma1(i) = sum(sum(quad1));
        suma2(i) = sum(sum(quad2));
        suma3(i) = sum(sum(quad3));
        suma4(i) = sum(sum(quad4));
        xa(i) = (-suma1(i)/r)+(-suma3(i)/r)+(suma2(i)/r)+(suma4(i)/r);
        ya(i) = (suma1(i)/r)+(suma2(i)/r)+(-suma3(i)/r)+(-suma4(i)/r);
    end
    
    figure(1);
    plot(x);
    figure(2);
    plot(y);
    figure(3);
    plot(xa);
    figure(4);
    plot(ya);
    figure(5);
    plot(x,y);
    figure(6);
    plot(xa,ya);
    
end
    
