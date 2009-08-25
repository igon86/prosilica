function createFit(x,y)
%CREATEFIT    Create plot of datasets and fits
%   CREATEFIT(X,Y)
%   Creates a plot, similar to the plot in the main curve fitting
%   window, using the data that you provide as input.  You can
%   apply this function to the same data you used with cftool
%   or with different data.  You may want to edit the function to
%   customize the code and this help message.
%
%   Number of datasets:  1
%   Number of fits:  1


% Data from dataset "y vs. x":
%    X = x:
%    Y = y:
%    Unweighted
%
% This function was automatically generated on 07-Aug-2009 17:11:04

% Set up figure to receive datasets and fits
f_ = clf;
figure(f_);
set(f_,'Units','Pixels','Position',[655 334 682 476]);
legh_ = []; legt_ = {};   % handles and text for legend
xlim_ = [Inf -Inf];       % limits of x axis
ax_ = axes;
set(ax_,'Units','normalized','OuterPosition',[0 0 1 1]);
set(ax_,'Box','on');
axes(ax_); hold on;


% --- Plot data originally in dataset "y vs. x"
x = x(:);
y = y(:);
h_ = line(x,y,'Parent',ax_,'Color',[0.333333 0 0.666667],...
    'LineStyle','none', 'LineWidth',1,...
    'Marker','.', 'MarkerSize',12);
xlim_(1) = min(xlim_(1),min(x));
xlim_(2) = max(xlim_(2),max(x));
legh_(end+1) = h_;
legt_{end+1} = 'y vs. x';

% Nudge axis limits beyond data limits
if all(isfinite(xlim_))
    xlim_ = xlim_ + [-1 1] * 0.01 * diff(xlim_);
    set(ax_,'XLim',xlim_)
else
    set(ax_, 'XLim',[-6.5099999999999997868, 759.50999999999999091]);
end


% --- Create fit "gaussian_fit"
fo_ = fitoptions('method','NonlinearLeastSquares','Lower',[-Inf -Inf    0]);
ok_ = isfinite(x) & isfinite(y);
if ~all( ok_ )
    warning( 'GenerateMFile:IgnoringNansAndInfs', ...
        'Ignoring NaNs and Infs in data' );
end
st_ = [45 407 98.980166962702270439 ];
set(fo_,'Startpoint',st_);
ft_ = fittype('gauss1');

% Fit this model using new data
cf_ = fit(x(ok_),y(ok_),ft_,fo_);

% Or use coefficients from the original fit:
if 0
    cv_ = { 22.135751045357014988, 292.19012002813650497, 568.3801514450282184};
    cf_ = cfit(ft_,cv_{:});
end

% Plot this fit
h_ = plot(cf_,'fit',0.95);
legend off;  % turn off legend from plot method call
set(h_(1),'Color',[1 0 0],...
    'LineStyle','-', 'LineWidth',2,...
    'Marker','none', 'MarkerSize',6);
legh_(end+1) = h_(1);
legt_{end+1} = 'gaussian_fit';

% Done plotting data and fits.  Now finish up loose ends.
hold off;
leginfo_ = {'Orientation', 'vertical', 'Location', 'NorthEast'};
h_ = legend(ax_,legh_,legt_,leginfo_{:});  % create legend
set(h_,'Interpreter','none');
xlabel(ax_,'');               % remove x label
ylabel(ax_,'');               % remove y label
