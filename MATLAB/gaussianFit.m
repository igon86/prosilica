%max,x_0,y_0 vengono dalla preelaborazione in C, sigma_x e sigma_y si
%possono ricavare dall'immagine come bordi della maschera (forse...)
function [A,x_0,y_0,sigma_x,sigma_y,a,b,c] = gaussianFit(image,max,min,center_x,center_y,var_x,var_y)
    
    %plot della roba da fittare
    figure(1);
    imag = double(image);
    mesh(imag);
    title('toBeFitted');
    
    % STOP CRITERIA parameter and iteration limit
    iteration_limit = 10;
    error_threshold = 100;
    
    %STATISTICAL parameters
    %percentage of error for the threshold
    stat_thres = 5;
    %area around the threshold
    stat_accuracy = 100;
    %number of points in the statistical plots
    stat_plot_precision = 100;
    
    %parameter inizialization
    A = max;
    x_0 = center_x;
    y_0 = center_y;
    sigma_x = var_x;
    sigma_y = var_y;
    a = 0;
    b = 0;
    c = min;
    
    %support data arrays inizialization
    [dimx,dimy] = size(image);
    img = imag(:);
    
    m=size(img,1);
    
    diff = zeros(m,1);
    immagine = zeros(m,1);
    M = zeros(m,8);
    
    
    %predition plot
    for i=1:m
        x = mod(i ,  dimx);
        y = floor(i/dimx);
        immagine(i) = evaluateGaussian(A,x_0,y_0,sigma_x,sigma_y,a,b,c,x,y);
    end
    predition = reshape(immagine,dimx,dimy);
    
    figure(2);
    mesh(predition);
    title('initialPredition (C++ RMS pre-elaboration)');
    drawnow;
    
    %initial error
    for i=1:m
            x = mod(i,  dimx);
            y = floor(i/dimx);
            test = evaluateGaussian(A,x_0,y_0,sigma_x,sigma_y,a,b,c,x,y);
%             if mask(i) == 1
                   diff(i) = img(i) - test;
%             end
           
    end
    
    R(1) = diff'*diff;
    initial_error = R(1);
    current_error = R(1);
    
    threshold = initial_error/error_threshold;
    %lo faccio girare per un po
    for j=1:iteration_limit
        %a questo punto calcolo il vettore delle diff e la matrice M

        for i=1:m
            x = mod(i,  dimx);
            y = floor(i/dimx);
            
            M(i,1) = 1/exp((x - x_0)^2/sigma_x^2 + (y - y_0)^2/sigma_y^2);
            M(i,2) = (A*(2*x - 2*x_0))/(sigma_x^2*exp((x - x_0)^2/sigma_x^2 + (y - y_0)^2/sigma_y^2));
            M(i,3) = (A*(2*y - 2*y_0))/(sigma_y^2*exp((x - x_0)^2/sigma_x^2 + (y - y_0)^2/sigma_y^2));
            M(i,4) = (2*A*(x - x_0)^2)/(sigma_x^3*exp((x - x_0)^2/sigma_x^2 + (y - y_0)^2/sigma_y^2));
            M(i,5) = (2*A*(y - y_0)^2)/(sigma_y^3*exp((x - x_0)^2/sigma_x^2 + (y - y_0)^2/sigma_y^2));
            %derivative of the slopePlan!
            M(i,6) = x;
            M(i,7) = y;
        end
        %derivative of the slopePlan!
        M(i,8) = 1;

        %prediction error plot
        if j == 1
            figure(6);
            diffmap = reshape(diff,dimx,dimy);
            mesh(diffmap);
            title('Predition residual map');
        end
        
        %calculating matrix and vector of the fit iteration
        matrix = M'*M;
        vector = M'*diff;

        %linear system resolution gives delta, vector of adjustments
        delta = matrix\vector;

        %adjusting results
        A = A+delta(1);
        x_0 = x_0 + delta(2);
        y_0 = y_0 + delta(3);
        sigma_x = sigma_x + delta(4);
        sigma_y = sigma_y + delta(5);
        a = a+delta(6);
        b = b+delta(7);
        c = c+delta(8);
        
        %ERROR RECALCULATION
        for i=1:m
            x = mod(i,  dimx);
            y = floor(i/dimx);
            test = evaluateGaussian(A,x_0,y_0,sigma_x,sigma_y,a,b,c,x,y);
%             if mask(i) == 1
                   diff(i) = img(i) - test;
%             end
            
        end
        
        R(j+1) = diff'*diff;
        
        %STOP CRITERIA
        
        if(R(j+1) > current_error)
            
           %not converging
           fprintf(1,'NOT CONVERGING..');
           if(R(j+1)>2*initial_error)
               fprintf(1,'REALLY BAD\n')
               iterated = j;
               break;
           else
               %fprintf(1,'SAFE\n');
               current_error = R(j+1);
           end
           
        else
            
            %converging
            if(abs(R(j+1) - current_error) < threshold)
                fprintf(1,'STOP AT ITERATION: %d\n',j);
                iterated = j;
                break;
            else
                current_error = R(j+1);
            end
            
        end
        
        iterated = j;
    end
    
    %%%%%%%%%%%%%%%%%%%%%%%%%     FINAL PLOTS    %%%%%%%%%%%%%%%%%%%%%%%%%
    
    %plot3D iterata
    for i=1:m
        x = mod(i ,  dimx);
        y = floor(i/dimx);
        immagine(i) = evaluateGaussian(A,x_0,y_0,sigma_x,sigma_y,a,b,c,x,y);
    end
    gaussiana = reshape(immagine,dimx,dimy);
    figure(3);
    mesh(gaussiana);
    title('fittedSolution');
    
   
    %stampo errore
    SSerr = R(iterated+1);
    
    %plot2D square residuals
    figure(4);
    if iterated < 10
        plot(linspace(0,iterated,iterated+1),R);
    else
        semilogy(linspace(0,iterated,iterated+1),R);
    end
    title(['R2 square residuals, R2 is: ',num2str(SSerr)]);    
    
    
    %calcolo SStot
    f_avg = mean(img);
    SStot = 0;
    for i = 1:m
        SStot = SStot + (img(i) - f_avg)^2;
    end
    %calcolo R2
    R2 = 1 - (SSerr/SStot);
    
    %plot(mappa residui)
    figure(5);
    %calcolo per l'ultima volta
    diff = img - immagine;
    diffmap = reshape(diff,dimx,dimy);
    mesh(diffmap);
    title(['Residuals map, goodness of fit is: ',num2str(R2)]);
    
    %%%%%%%%%%%%%%%%%%%%%%%%%     STATISTICS  x_0    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    
    
    %confidence of x
    
    threshold = SSerr*((100+stat_thres)/100);
    epsilon = SSerr/stat_accuracy;
    upthreshold = threshold + epsilon;
    lowthreshold = threshold - epsilon;
    
    %left border -> algoritmo di bisezione
    
    left = 0;
    right = x_0;
    center = x_0/2;
    quadrati = 2*SSerr;
    
    iterazione=1;
    
    while quadrati > upthreshold || quadrati < lowthreshold
        %fprintf(1,'Iterazione %d con residuo %d che lavora su %d   %d  %d\n',iterazione,quadrati,left,center,right);
        %adjustment of the section
        if quadrati > upthreshold
            left = center;
            center = left + ((right - left)/2);
        else
            right = center;
            center = left + ((right - left)/2);
        end
        
        %new error calculation
        for i=1:m
            x = mod(i ,  dimx);
            y = floor(i/dimx);
            immagine(i) = evaluateGaussian(A,center,y_0,sigma_x,sigma_y,a,b,c,x,y);
        end
        
        diff = img - immagine;
        quadrati = diff' * diff;
        
        iterazione = iterazione +1;
    end
    
    left_confidence_x = center;
    left_spot_x = quadrati;
    
    fprintf(1,'Analyzing X_0\n');
    
    %right border -> algoritmo di bisezione
    
    left = x_0;
    right = dimx;
    center = left + (right-left)/2;
    quadrati = 2*SSerr;
    
    
    
    while quadrati > upthreshold || quadrati < lowthreshold
        %fprintf(1,'Iterazione %d con residuo %d che lavora su %d   %d  %d\n',iterazione,quadrati,left,center,right);
        %adjustment of the section
        if quadrati > upthreshold
            right = center;
            center = left + (right - left)/2;
        else
            left = center;
            center = left + (right - left)/2;
        end
        
        %new error calculation
        for i=1:m
            x = mod(i ,  dimx);
            y = floor(i/dimx);
            immagine(i) = evaluateGaussian(A,center,y_0,sigma_x,sigma_y,a,b,c,x,y);
        end
        
        diff = img - immagine;
        quadrati = diff' * diff;
    end
    
    right_confidence_x = center;
    right_spot_x = quadrati;
    
    % plot su x
    s = 100;
    x_s = linspace(x_0 -10,x_0 +10,s);
    
    stat_x = zeros(s,1);
    for j=1:s
        %calcolo l'immagine shiftata
        for i=1:m
            x = mod(i ,  dimx);
            y = floor(i/dimx);
            immagine(i) = evaluateGaussian(A,x_s(j),y_0,sigma_x,sigma_y,a,b,c,x,y);
        end
        
        diff = img - immagine;
        quadrati = diff' * diff;
        stat_x(j) = quadrati;
        %         if(quadrati < R2)
        %             fprintf('Non ha convergiuto completamente, R: %d',quadrati);
        %         end
    end
    
    x_0_width = right_confidence_x - left_confidence_x;
    
     figure(7);
     plot(x_s,stat_x,left_confidence_x,left_spot_x,'ro',right_confidence_x,right_spot_x,'ro')
     title({['X_0 is: ',num2str(x_0),' and is between ',num2str(left_confidence_x),' and ',num2str(right_confidence_x),' with ',num2str(x_0_width),' width.'];;});
    

    
    
    
%%%%%%%%%%%%%%%%%%%%%%%%%     STATISTICS  y_0    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%    
    
    
    
    %confidence of y
    
    stat_thres = 5;
    threshold = SSerr*((100+stat_thres)/100);
    epsilon = SSerr/100;
    upthreshold = threshold + epsilon;
    lowthreshold = threshold - epsilon;
    
    %left border -> algoritmo di bisezione
    
    left = 0;
    right = y_0;
    center = y_0/2;
    quadrati = 2*SSerr;
    
    iterazione=1;
    
    while quadrati > upthreshold || quadrati < lowthreshold
        %fprintf(1,'Iterazione %d con residuo %d che lavora su %d   %d  %d\n',iterazione,quadrati,left,center,right);
        %adjustment of the section
        if quadrati > upthreshold
            left = center;
            center = left + ((right - left)/2);
        else
            right = center;
            center = left + ((right - left)/2);
        end
        
        %new error calculation
        for i=1:m
            x = mod(i ,  dimx);
            y = floor(i/dimx);
            immagine(i) = evaluateGaussian(A,x_0,center,sigma_x,sigma_y,a,b,c,x,y);
        end
        
        diff = img - immagine;
        quadrati = diff' * diff;
        
        iterazione = iterazione +1;
    end
    
    left_confidence_y = center;
    left_spot_y = quadrati;
    
    fprintf(1,'Analyzing Y_0\n');
    
    %right border -> algoritmo di bisezione
    
    left = y_0;
    right = dimy;
    center = left + (right-left)/2;
    quadrati = 2*SSerr;
    
    
    while quadrati > upthreshold || quadrati < lowthreshold
        %fprintf(1,'Iterazione %d con residuo %d che lavora su %d   %d  %d\n',iterazione,quadrati,left,center,right);
        %adjustment of the section
        if quadrati > upthreshold
            right = center;
            center = left + (right - left)/2;
        else
            left = center;
            center = left + (right - left)/2;
        end
        
        %new error calculation
        for i=1:m
            x = mod(i ,  dimx);
            y = floor(i/dimx);
            immagine(i) = evaluateGaussian(A,x_0,center,sigma_x,sigma_y,a,b,c,x,y);
        end
        
        diff = img - immagine;
        quadrati = diff' * diff;
    end
    
    right_confidence_y = center;
    right_spot_y = quadrati;
    
    % plot su y
    y_s = linspace(y_0 -10,y_0 +10,stat_plot_precision);
    
    stat_y = zeros(stat_plot_precision,1);
    for j=1:s
        %calcolo l'immagine shiftata
        for i=1:m
            x = mod(i ,  dimx);
            y = floor(i/dimx);
            immagine(i) = evaluateGaussian(A,x_0,y_s(j),sigma_x,sigma_y,a,b,c,x,y);
        end
        
        diff = img - immagine;
        quadrati = diff' * diff;
        stat_y(j) = quadrati;
        %         if(quadrati < R2)
        %             fprintf('Non ha convergiuto completamente, R: %d',quadrati);
        %         end
    end
    
    y_0_width = right_confidence_y - left_confidence_y;
    
    figure(8);
    plot(y_s,stat_y,left_confidence_y,left_spot_y,'ro',right_confidence_y,right_spot_y,'ro')
    title({['Y_0 is: ',num2str(y_0),' and is between ',num2str(left_confidence_y),' and ',num2str(right_confidence_y),' with ',num2str(y_0_width),' width.\linebreak'];''});
    
%%%%%%%%%%%%%%%%%%%%%%%%%     STATISTICS  sigma_x    %%%%%%%%%%%%%%%%%%%%%%%%%

    %left border -> algoritmo di bisezione
    
    left = 0;
    right = sigma_x;
    center = left + (right-left)/2 ;
    quadrati = 2*SSerr;
    
    iterazione=1;
    
    while quadrati > upthreshold || quadrati < lowthreshold

        %adjustment of the section
        
        if quadrati > upthreshold
            left = center;
            center = left + ((right - left)/2);
        else
            right = center;
            center = left + ((right - left)/2);
        end
        
        %new error calculation
        for i=1:m
            x = mod(i ,  dimx);
            y = floor(i/dimx);
            immagine(i) = evaluateGaussian(A,x_0,y_0,center,sigma_y,a,b,c,x,y);
        end
        
        diff = img - immagine;
        quadrati = diff' * diff;
        
        iterazione = iterazione +1;
    end
    
    left_confidence_sigma_x = center;
    left_spot_sigma_x = quadrati;
    
    
    fprintf(1,'Analyzing sigma_x\n');
    
    %right border -> algoritmo di bisezione
    
    left = sigma_x;
    right = 10*sigma_x;
    center = left + (right-left)/2;
    quadrati = 2*SSerr;
    
    
    while quadrati > upthreshold || quadrati < lowthreshold

        %adjustment of the section
        if quadrati > upthreshold
            right = center;
            center = left + (right - left)/2;
        else
            left = center;
            center = left + (right - left)/2;
        end
        
        %new error calculation
        for i=1:m
            x = mod(i ,  dimx);
            y = floor(i/dimx);
            immagine(i) = evaluateGaussian(A,x_0,y_0,center,sigma_y,a,b,c,x,y);
        end
        
        
        diff = img - immagine;
        quadrati = diff' * diff;
    end
    
    right_confidence_sigma_x = center;
    right_spot_sigma_x = quadrati;
    
    % plot su x
    sigma_x_s = linspace(sigma_x -10,sigma_x +10,stat_plot_precision);
    
    stat_sigma_x = zeros(stat_plot_precision,1);
    for j=1:stat_plot_precision
        %calcolo l'immagine shiftata
        for i=1:m
            x = mod(i ,  dimx);
            y = floor(i/dimx);
            immagine(i) = evaluateGaussian(A,x_0,y_0,sigma_x_s(j),sigma_y,a,b,c,x,y);
        end
        
        diff = img - immagine;
        quadrati = diff' * diff;
        stat_sigma_x(j) = quadrati;
    end
    
    figure(9);
    plot(sigma_x_s,stat_sigma_x,left_confidence_sigma_x,left_spot_sigma_x,'ro',right_confidence_sigma_x,right_spot_sigma_x,'ro')
    title({[' sigma_x is: ',num2str(sigma_x),' and is between',num2str(left_confidence_sigma_x),' and ',num2str(right_confidence_sigma_x)],});

    
    
    
    %%%%%%%%%%%%%%%%%%%%%%%%%     STATISTICS  sigma_y    %%%%%%%%%%%%%%%%%%%%%%%%%

    %left border -> algoritmo di bisezione
    
    left = 0;
    right = sigma_y;
    center = left + (right-left)/2 ;
    quadrati = 2*SSerr;
    
    iterazione=1;
    
    while quadrati > upthreshold || quadrati < lowthreshold

        %adjustment of the section
        
        if quadrati > upthreshold
            left = center;
            center = left + ((right - left)/2);
        else
            right = center;
            center = left + ((right - left)/2);
        end
        
        %new error calculation
        for i=1:m
            x = mod(i ,  dimx);
            y = floor(i/dimx);
            immagine(i) = evaluateGaussian(A,x_0,y_0,sigma_x,center,a,b,c,x,y);
        end
        
        diff = img - immagine;
        quadrati = diff' * diff;
        
        iterazione = iterazione +1;
    end
    
    left_confidence_sigma_y = center;
    left_spot_sigma_y = quadrati;
    
    
    fprintf(1,'Analyzing sigma_y\n');
    
    %right border
    
    left = sigma_y;
    right = 10*sigma_y;
    center = left + (right-left)/2;
    quadrati = 2*SSerr;
    
    iterazione=1;
    
    while quadrati > upthreshold || quadrati < lowthreshold

        %adjustment of the section
        if quadrati > upthreshold
            right = center;
            center = left + (right - left)/2;
        else
            left = center;
            center = left + (right - left)/2;
        end
        
        %new error calculation
        for i=1:m
            x = mod(i ,  dimx);
            y = floor(i/dimx);
            immagine(i) = evaluateGaussian(A,x_0,y_0,sigma_x,center,a,b,c,x,y);
        end
        
        
        diff = img - immagine;
        quadrati = diff' * diff;
    end
    
    right_confidence_sigma_y = center;
    right_spot_sigma_y = quadrati;
    
    % plot OF SIGMA_Y
    sigma_y_s = linspace(sigma_y -10,sigma_y +10,stat_plot_precision);
    
    stat_sigma_y = zeros(stat_plot_precision,1);
    for j=1:stat_plot_precision
        %calcolo l'immagine shiftata
        for i=1:m
            x = mod(i ,  dimx);
            y = floor(i/dimx);
            immagine(i) = evaluateGaussian(A,x_0,y_0,sigma_x,sigma_y_s(j),a,b,c,x,y);
        end
        
        diff = img - immagine;
        quadrati = diff' * diff;
        stat_sigma_y(j) = quadrati;

    end
    
    figure(10);
    plot(sigma_y_s,stat_sigma_y,left_confidence_sigma_y,left_spot_sigma_y,'ro',right_confidence_sigma_y,right_spot_sigma_y,'ro')
    title({[' sigma_x is: ',num2str(sigma_y),' and is between',num2str(left_confidence_sigma_y),' and ',num2str(right_confidence_sigma_y)],});
end
