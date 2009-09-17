%max,x_0,y_0 vengono dalla preelaborazione in C, sigma_x e sigma_y si
%possono ricavare dall'immagine come bordi della maschera (forse...)
function [A,x_0,y_0,sigma_x,sigma_y,a,b,c] = gaussianFit(image,max,min,centro_x,centro_y,var_x,var_y)
    
    %plot della roba da fittare
    figure(1);
    imag = double(image);
    mesh(imag);
    title('toBeFitted');
    
    % stop criteria parameter and iteration limit
    iteration_limit = 10;
    error_threshold = 100;
    
    %parameter inizialization
    A = max;
    x_0 = centro_x;
    y_0 = centro_y;
    sigma_x = var_x;
    sigma_y = var_y;
    a = 0;
    b = 0;
    c = min;
    
    %support data arrays inizialization
    [dimx,dimy] = size(image);
    img = imag(:);
    
    m=size(img,1);
    
    differenze = zeros(m,1);
    immagine = zeros(m,1);
    M = zeros(m,8);
    
    
    %predition plot
    for i=1:m
        x = mod(i ,  dimx);
        y = floor(i/dimx);
        immagine(i) = valutaPunto(A,x_0,y_0,sigma_x,sigma_y,a,b,c,x,y);
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
            test = valutaPunto(A,x_0,y_0,sigma_x,sigma_y,a,b,c,x,y);
%             if mask(i) == 1
                   differenze(i) = img(i) - test;
%             end
           
    end
    
    R(1) = differenze'*differenze;
    initial_error = R(1);
    current_error = R(1);
    
    threshold = initial_error/error_threshold;
    %lo faccio girare per un po
    for j=1:iteration_limit
        %a questo punto calcolo il vettore delle differenze e la matrice M

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
            diffmap = reshape(differenze,dimx,dimy);
            mesh(diffmap);
            title('Predition residual map');
        end
        
        %calculating matrix and vector of the fit iteration
        matrix = M'*M;
        vector = M'*differenze;

        %calculating 
        R(j+1) = differenze'*differenze;

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
            test = valutaPunto(A,x_0,y_0,sigma_x,sigma_y,a,b,c,x,y);
%             if mask(i) == 1
                   differenze(i) = img(i) - test;
%             end
            
        end
        
        R(j+1) = differenze'*differenze;
        
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
        
    end
    
    
    
    %plot3D iterata
    for i=1:m
        x = mod(i ,  dimx);
        y = floor(i/dimx);
        immagine(i) = valutaPunto(A,x_0,y_0,sigma_x,sigma_y,a,b,c,x,y);
    end
    gaussiana = reshape(immagine,dimx,dimy);
    figure(3);
    mesh(gaussiana);
    title('fittedSolution');
    
   
    %stampo errore
    R2 = R(iterated+1);
    
    %plot2D residui
    figure(4);
    plot(linspace(0,iterated,iterated+1),R);
    title(['R2 square residuals, R2 is: ',num2str(R2)]);    
    
    %plot(mappa residui)
    figure(5);
    %calcolo per l'ultima volta
    differenze = img - immagine;
    diffmap = reshape(differenze,dimx,dimy);
    mesh(diffmap);
    title('Residua map');
end