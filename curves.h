#ifndef CURVES_H
#define CURVES_H

#include <bevgrafmath2017.h>
#include <vector>
#include <QOpenGLWidget>
using namespace std;

//////////////////BEZIER6////////////////////////
vec2 casteljau(double t,vector<vec2> points)
{
    for(int i = 1; i < points.size(); i++)
        for(int j = 0; j < points.size() - i; j++) {
            points[j].x = (1 - t) * points[j].x + t * points[j + 1].x;
            points[j].y = (1 - t) * points[j].y + t * points[j + 1].y;
        }

    return points[0];
}

void bezier(vector<vec2> ControlPoints,vector<vec2>& output){
    output.clear();
    for(GLfloat t = 0; t <= 1; t += 0.01) {
        output.emplace_back(casteljau(t,ControlPoints));
    }

}

//TODO optimise rationalBezier
///////////////////////rationalBezier//////////////////////////////

int n_under_k(int n, int k)
   {
     // Base Cases
     if (k==0 || k==n)
       return 1;
       int res = 1;

     // Recur
     return  n_under_k(n-1, k-1) + n_under_k(n-1, k);
}

    GLfloat Berstein(GLfloat t, int i, int k) {
        return n_under_k(k, i) * pow(t, i)*pow((1 - t), (k - i));
    }

    vec2 calculateRationalBezierPoint(vector<vec2> ControlPoints, vector<GLdouble> weight ,GLfloat t) {
        int n = ControlPoints.size();
        vec2 Point = {0,0};
        GLfloat divisor = 0;

        for (int i = 0; i < n; i++) {
            divisor += weight[i] * Berstein(t, i, (n-1));
        }

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < 2; j++)
                Point[j] += ( weight[i] * ControlPoints[i][j] * Berstein(t, i, (n-1)) ) ;
        }

        Point[0] /= divisor;
        Point[1] /= divisor;

        return Point;
    }

    void rationalBezier(vector<vec2> ControlPoints,vector<GLdouble> weight,vector<vec2>& output){
        output.clear();
        for (GLfloat t = 0; t <= 1; t += 0.01)
        {
            vec2 Point = calculateRationalBezierPoint(ControlPoints,weight,t);
            output.push_back(Point);
        }
    }
////////////////////////////////////BSPLINE/////////////////////////////////////////////
    float GetPoint(int i,int pointSize) {
        // return 1st point
        if (i<0) {
            return 0;
        }
        // return last point
        if (i<pointSize)
            return i;

        return pointSize - 1;
    }

    void bSpline(vector<vec2> points,vector<vec2> & output, vector<vec2> &breakPoints) {
        output.clear();
        breakPoints.clear();
        float pointSize = points.size();
        int lod = 20;
        vec2 curvepoint;

        for (int start_cv = -3, j = 0;j != pointSize+1;++j, ++start_cv) {

            for (int i = 0;i <= lod;++i) {

                float t = (float)i / lod;
                // inverted t
                float it = 1.0f - t;

                float b0 = it * it*it / 6.0f;
                float b1 = (3 * t*t*t - 6 * t*t + 4) / 6.0f;
                float b2 = (-3 * t*t*t + 3 * t*t + 3 * t + 1) / 6.0f;
                float b3 = t * t*t / 6.0f;

                curvepoint.x = b0 * points[GetPoint(start_cv + 0,points.size())].x +
                    b1 * points[GetPoint(start_cv + 1,points.size())].x +
                    b2 * points[GetPoint(start_cv + 2,points.size())].x +
                    b3 * points[GetPoint(start_cv + 3,points.size())].x;

                curvepoint.y = b0 * points[GetPoint(start_cv + 0,points.size())].y +
                    b1 * points[GetPoint(start_cv + 1,points.size())].y +
                    b2 * points[GetPoint(start_cv + 2,points.size())].y +
                    b3 * points[GetPoint(start_cv + 3,points.size())].y;

                output.emplace_back(curvepoint);
            }

            breakPoints.emplace_back(curvepoint);
        }
        output.emplace_back(vec2(points[pointSize - 1].x, points[pointSize - 1].y));
    }


//////////////////////////rational_bspline_curve////////////////////////////////
    void rationalBspline(std::vector<vec2> controlPoints, std::vector<GLdouble> weights,std::vector<vec2>& output,std::vector<vec2>& breakPoints) {
        output.clear();
        breakPoints.clear();
        controlPoints.insert(controlPoints.begin(), controlPoints[0]);
        controlPoints.insert(controlPoints.begin(), controlPoints[0]);
        weights.insert(weights.begin(), weights[0]);
        weights.insert(weights.begin(), weights[0]);

        controlPoints.push_back(controlPoints.back());
        controlPoints.push_back(controlPoints.back());
        weights.push_back(weights.back());
        weights.push_back(weights.back());


        GLfloat sum = 0;
        GLfloat M[4][4] = { { (-1.0 / 6.0), (3.0 / 6.0), (-3.0 / 6.0), (1.0 / 6.0) },
                            { (3.0 / 6.0), -1.0, 0, (4.0 / 6.0) },
                            { ((-3.0 / 6.0)), (3.0 / 6.0), (3.0 / 6.0), (1.0 / 6.0) },
                            { (1.0 / 6.0), 0, 0, 0 } };

        GLint controlPointsSize = controlPoints.size();
        for (int i = 0; i < controlPointsSize; i++) {
            controlPoints[i] *= weights[i];
        }
        vec2 currentPoint;
        for (int i = 0; i < controlPointsSize - 3; i++) {

            for (GLfloat t = 0; t <= 1; t += 0.01) {

                GLfloat T[4] = { t*t*t,t*t,t,1 };
                GLfloat MT[4];

                for (int j = 0; j < 4; j++)
                {
                    sum = 0;
                    for (int k = 0; k < 4; k++)
                        sum = sum + M[j][k] * T[k];
                    MT[j] = sum;

                }
                float
                    sum = 0;
                for (int j = 0; j < 4; j++)
                    sum += MT[j] * weights[i + j];
                GLfloat rationalSum = sum;
                sum = 0;

                for (int j = 0; j < 2; j++)
                {
                    sum = 0;
                    for (int k = 0; k < 4; k++)
                        sum += controlPoints[i + k][j] * MT[k];
                    currentPoint[j] = sum/rationalSum;
                }
                output.push_back(currentPoint);



            }

            breakPoints.push_back({currentPoint[0],currentPoint[1]});
        }

    }


 //////////////////////////NURBS////////////////////////////////
    float CoxDeBoor(float u, int i, int k, vector<float> Knots) {
        if (k == 1)
        {
            if (Knots[i] <= u && u <= Knots[i + 1]) {
                return 1.0f;
            }
            return 0.0f;
        }
        float Den1 = Knots[i + k - 1] - Knots[i];
        float Den2 = Knots[i + k] - Knots[i + 1];
        float Eq1 = 0, Eq2 = 0;
        if (Den1 > 0) {
            Eq1 = ((u - Knots[i]) / Den1) * CoxDeBoor(u, i, k - 1, Knots);
        }
        if (Den2 > 0) {
            Eq2 = (Knots[i + k] - u) / Den2 * CoxDeBoor(u, i + 1, k - 1, Knots);
        }
        return Eq1 + Eq2;
    }


    void calculateNurbsKnots(int g_degree, int controlPointsSize, std::vector<float> &knots){
        knots.clear();
        for (int i = 0; i<g_degree+1; i++) {
                knots.insert(knots.begin(),0.0f);
                knots.insert(knots.end(),100.0f);
        }

        int customKnots = (controlPointsSize - g_degree - 1);
        float inc = floor(((100.0f/(customKnots+1)) * 100) + .5) / 100;

        for (int i = 0;i<customKnots;i++) {
            knots.insert(knots.begin()+(g_degree+1+i),knots[g_degree+i] + inc);
        }

    }

    void nurbs(std::vector<vec2> controlPoints, std::vector<float> knots,std::vector<vec2>& output){
        output.clear();
        unsigned int g_num_cvs = controlPoints.size();
        unsigned int g_degree = 3;
        unsigned int g_order = g_degree + 1;
        unsigned int g_num_knots = g_num_cvs + g_order;
        unsigned int LOD = 100;
        float t = 0;

        for(int i = 0; i < LOD; ++i){
            t  = knots[g_num_knots-1] * i / (float)(LOD-1);
            if(i==LOD-1)
                t-=0.001f;

            float Outpoint[2] = {0, 0};

            for (unsigned int j = 0; j != g_num_cvs; ++j) {

                // calculate the effect of this point on the curve
                float Val = CoxDeBoor(t, j, g_order, knots);

                if (Val > 0.001f) {

                    // sum effect of CV on this part of the curve
                    Outpoint[0] += Val * controlPoints[j].x;
                    Outpoint[1] += Val * controlPoints[j].y;
                }
            }

            output.push_back({Outpoint[0],Outpoint[1]});
       }
    }

#endif // CURVES_H
