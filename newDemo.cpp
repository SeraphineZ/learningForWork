#include <vtkMatrix4x4.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkLandmarkTransform.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkIterativeClosestPointTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkLandmarkTransform.h> //to set type to ridgid body
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <iostream>
vtkPolyData *CreatePolyData();

vtkPolyData *PerturbPolyData(vtkPolyData *polydata);
int main()
{
    vtkPolyData *TargetPolydata = CreatePolyData(); // 创建目标坐标系内的点集

    // 创建源坐标系内的点，实际上是通过给目标坐标系内点集加一个扰动实现的
    vtkPolyData *SourcePolydata = PerturbPolyData(TargetPolydata);

    // 开始用vtkIterativeClosestPointTransform类实现 ICP算法
    vtkIterativeClosestPointTransform *icp = vtkIterativeClosestPointTransform::New();
    icp->SetSource(SourcePolydata);
    icp->SetTarget(TargetPolydata);
    icp->GetLandmarkTransform()->SetModeToRigidBody();
    icp->SetMaximumNumberOfIterations(20);
    icp->StartByMatchingCentroidsOn();
    icp->Modified();
    icp->Update();

    vtkMatrix4x4 *M = icp->GetMatrix();
    std::cout << "The resulting matrix is: " << *M << endl;
    // 以下是为更方便地显示矩阵，统一了矩阵内数字显示形式，矩阵内数字形如：1.08e-001
    for (int i = 0; i <= 3; i++)
    {
        printf("\n");
        for (int j = 0; j <= 3; j++)
        {
            printf("%e\t", M->Element[i][j]);
        }
    }
    SourcePolydata->Delete();
    TargetPolydata->Delete();
    getchar();
    return 0;
}
vtkPolyData *CreatePolyData()
{
    // This function creates a set of 5 points (the origin and a point unit distance along each axis)
    vtkPoints *SourcePoints = vtkPoints::New();
    vtkCellArray *SourceVertices = vtkCellArray::New();
    // create three points and create vertices out of them
    vtkIdType pid[1]; // 记录下一个要加入的点在vtkPoints 中存储序号
    double Origin[3] = {0.0, 0.0, 0.0};
    pid[0] = SourcePoints->InsertNextPoint(Origin);
    SourceVertices->InsertNextCell(1, pid);
    double SourcePoint1[3] = {1.0, 0.0, 0.0};
    pid[0] = SourcePoints->InsertNextPoint(SourcePoint1);
    SourceVertices->InsertNextCell(1, pid);
    double SourcePoint2[3] = {0.0, 1.0, 0.0};
    pid[0] = SourcePoints->InsertNextPoint(SourcePoint2);
    SourceVertices->InsertNextCell(1, pid);
    double SourcePoint3[3] = {1.0, 1.0, 0.0}; //{0.0, 0.0, 1.0};
    pid[0] = SourcePoints->InsertNextPoint(SourcePoint3);
    SourceVertices->InsertNextCell(1, pid);
    double SourcePoint4[3] = {0.5, 0.5, 0.0}; //{0.0, 0.0, 1.0};
    pid[0] = SourcePoints->InsertNextPoint(SourcePoint4);
    SourceVertices->InsertNextCell(1, pid);
    vtkPolyData *polydata = vtkPolyData::New();
    polydata->SetPoints(SourcePoints); // 把点导入的polydata中去
    polydata->SetVerts(SourceVertices);
    return polydata;
}