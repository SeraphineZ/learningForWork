#include <vtkMatrix4x4.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkLandmarkTransform.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkIterativeClosestPointTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkLandmarkTransform.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkVertex.h>
#include <vtkSmartPointer.h>
#include <vtkProperty.h>
#include <vtkTransform.h>
#include <iostream>

vtkPolyData *CreatePolyData();
vtkPolyData *PerturbPolyData(vtkPolyData *polydata);
vtkPolyData *TransformPolyData(vtkPolyData *sourcePolydata, vtkMatrix4x4 *matrix);
vtkPolyData *ApplyRandomOffset(vtkPolyData *polydata); // 新增函数，给点集添加随机偏移
void DisplayPolyData(vtkPolyData *targetPolydata, vtkPolyData *sourcePolydata, vtkPolyData *alignedPolydata);

/**
 * @brief 使用 ICP 算法配准源点集和目标点集，
 *        并在窗口中显示目标点集、源点集和配准后的点集
 *
 * @return 0 if success
 */
int main()
{
    vtkPolyData *TargetPolydata = CreatePolyData();                // 创建目标点集
    vtkPolyData *SourcePolydata = PerturbPolyData(TargetPolydata); // 创建源点集

    // 使用 ICP 算法配准
    vtkSmartPointer<vtkIterativeClosestPointTransform> icp = vtkSmartPointer<vtkIterativeClosestPointTransform>::New();
    icp->SetSource(SourcePolydata);
    icp->SetTarget(TargetPolydata);
    icp->GetLandmarkTransform()->SetModeToRigidBody();
    icp->SetMaximumNumberOfIterations(20);
    icp->StartByMatchingCentroidsOn();
    icp->Update();

    vtkMatrix4x4 *matrix = icp->GetMatrix();
    std::cout << "ICP Transformation Matrix:" << std::endl;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            std::cout << matrix->GetElement(i, j) << " ";
        }
        std::cout << std::endl;
    }

    // 将配准矩阵应用于源点集，生成配准后的点集
    vtkPolyData *AlignedPolydata = TransformPolyData(SourcePolydata, matrix);

    // 给源点集、目标点集和配准后的点集添加随机偏移，避免重叠
    TargetPolydata = ApplyRandomOffset(TargetPolydata);
    SourcePolydata = ApplyRandomOffset(SourcePolydata);
    AlignedPolydata = ApplyRandomOffset(AlignedPolydata);

    // 在窗口中显示目标点集、源点集和配准后的点集
    DisplayPolyData(TargetPolydata, SourcePolydata, AlignedPolydata);

    SourcePolydata->Delete();
    TargetPolydata->Delete();
    AlignedPolydata->Delete();

    return 0;
}

/**
 * @brief 该函数创建一个包含5个点的点集
 *
 * 该函数返回一个包含5个点的点集，点集的点坐标如下:
 * (0,0,0), (1,0,0), (0,1,0), (1,1,0), (0.5,0.5,0)
 *
 * @return 一个包含5个点的点集
 */
vtkPolyData *CreatePolyData()
{
    // 此函数创建一个包含5个点的点集
    vtkPoints *SourcePoints = vtkPoints::New();
    vtkCellArray *SourceVertices = vtkCellArray::New();
    vtkIdType pid[1];

    double Origin[3] = {0.0, 0.0, 0.0};
    pid[0] = SourcePoints->InsertNextPoint(Origin);
    SourceVertices->InsertNextCell(1, pid);

    double SourcePoint1[3] = {1.0, 0.0, 0.0};
    pid[0] = SourcePoints->InsertNextPoint(SourcePoint1);
    SourceVertices->InsertNextCell(1, pid);

    double SourcePoint2[3] = {0.0, 1.0, 0.0};
    pid[0] = SourcePoints->InsertNextPoint(SourcePoint2);
    SourceVertices->InsertNextCell(1, pid);

    double SourcePoint3[3] = {1.0, 1.0, 0.0};
    pid[0] = SourcePoints->InsertNextPoint(SourcePoint3);
    SourceVertices->InsertNextCell(1, pid);

    double SourcePoint4[3] = {0.5, 0.5, 0.0};
    pid[0] = SourcePoints->InsertNextPoint(SourcePoint4);
    SourceVertices->InsertNextCell(1, pid);

    vtkPolyData *polydata = vtkPolyData::New();
    polydata->SetPoints(SourcePoints); // 把点导入polydata中
    polydata->SetVerts(SourceVertices);

    return polydata;
}

/**
 * @brief 对输入的点集进行随机扰动，避免点集重叠
 *
 * @param[in] OldPolydata 输入的点集
 * @return 扰动后的点集
 */
vtkPolyData *PerturbPolyData(vtkPolyData *OldPolydata)
{
    vtkPolyData *polydata = vtkPolyData::New();
    polydata->DeepCopy(OldPolydata);
    vtkPoints *Points = polydata->GetPoints();

    double p[3];
    Points->GetPoint(1, p);
    p[0] = sqrt(2.0) / 2.0;
    p[2] = sqrt(2.0) / 2.0;
    Points->SetPoint(1, p);

    Points->GetPoint(3, p);
    p[0] = sqrt(2.0) / 2.0;
    p[2] = sqrt(2.0) / 2.0;
    Points->SetPoint(3, p);

    Points->GetPoint(4, p);
    p[0] = sqrt(2.0) / 4.0;
    p[2] = sqrt(2.0) / 4.0;
    Points->SetPoint(4, p);

    return polydata;
}

/**
 * @brief 将点集中的每个点都进行小的随机偏移，避免重叠
 *
 * 该函数遍历输入点集中每个点，并在x和y方向上添加小的随机偏移，避免点重叠。
 * 偏移量的绝对值小于0.05。
 *
 * @param[in] polydata 输入点集
 * @return 带有随机偏移的点集
 */
vtkPolyData *ApplyRandomOffset(vtkPolyData *polydata)
{
    // 给点集应用小的随机偏移，避免重叠
    vtkPoints *points = polydata->GetPoints();
    for (vtkIdType i = 0; i < points->GetNumberOfPoints(); i++)
    {
        double p[3];
        points->GetPoint(i, p);
        // 在x和y方向上添加小的随机偏移
        p[0] += (rand() % 1000) / 1000.0 * 0.05; // 偏移量为0.05
        p[1] += (rand() % 1000) / 1000.0 * 0.05; // 偏移量为0.05
        points->SetPoint(i, p);
    }
    return polydata;
}

/**
 * @brief 对源点云应用仿射变换矩阵。
 *
 * 该函数将源vtkPolyData作为输入，并将给定的仿射变换矩阵应用于生成一个新的变换后的vtkPolyData。
 *
 * @param[in] sourcePolydata 输入点云以进行仿射变换。
 * @param[in] matrix 仿射变换矩阵。
 * @return 一个新的vtkPolyData，包含变换后的点。
 */
vtkPolyData *TransformPolyData(vtkPolyData *sourcePolydata, vtkMatrix4x4 *matrix)
{
    vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
    transform->SetMatrix(matrix);

    vtkSmartPointer<vtkTransformPolyDataFilter> filter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    filter->SetInputData(sourcePolydata);
    filter->SetTransform(transform);
    filter->Update();

    vtkPolyData *transformedPolydata = vtkPolyData::New();
    transformedPolydata->DeepCopy(filter->GetOutput());
    return transformedPolydata;
}

/**
 * @brief 在窗口中显示目标点集、源点集和配准后的点集。
 *
 * 该函数创建一个渲染窗口，并将目标点集、源点集和配准后的点集以不同颜色渲染出来。
 * 目标点集显示为蓝色，源点集显示为红色，配准后的点集显示为绿色。
 * 所有点集的点大小均设置为10。
 *
 * @param[in] targetPolydata 目标点集，用于渲染的vtkPolyData。
 * @param[in] sourcePolydata 源点集，用于渲染的vtkPolyData。
 * @param[in] alignedPolydata 配准后的点集，用于渲染的vtkPolyData。
 */
void DisplayPolyData(vtkPolyData *targetPolydata, vtkPolyData *sourcePolydata, vtkPolyData *alignedPolydata)
{
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->AddRenderer(renderer);

    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    renderWindowInteractor->SetRenderWindow(renderWindow);

    // 配置目标点集（蓝色）
    vtkSmartPointer<vtkPolyDataMapper> targetMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    targetMapper->SetInputData(targetPolydata);
    vtkSmartPointer<vtkActor> targetActor = vtkSmartPointer<vtkActor>::New();
    targetActor->SetMapper(targetMapper);
    targetActor->GetProperty()->SetColor(0, 0, 1);
    targetActor->GetProperty()->SetPointSize(10);

    // 配置源点集（红色）
    vtkSmartPointer<vtkPolyDataMapper> sourceMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    sourceMapper->SetInputData(sourcePolydata);
    vtkSmartPointer<vtkActor> sourceActor = vtkSmartPointer<vtkActor>::New();
    sourceActor->SetMapper(sourceMapper);
    sourceActor->GetProperty()->SetColor(1, 0, 0);
    sourceActor->GetProperty()->SetPointSize(10);

    // 配置配准后的点集（绿色）
    vtkSmartPointer<vtkPolyDataMapper> alignedMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    alignedMapper->SetInputData(alignedPolydata);
    vtkSmartPointer<vtkActor> alignedActor = vtkSmartPointer<vtkActor>::New();
    alignedActor->SetMapper(alignedMapper);
    alignedActor->GetProperty()->SetColor(0, 1, 0);
    alignedActor->GetProperty()->SetPointSize(10);

    // 添加到渲染器
    renderer->AddActor(targetActor);
    renderer->AddActor(sourceActor);
    renderer->AddActor(alignedActor);

    // 设置背景和渲染
    renderer->SetBackground(1, 1, 1);
    renderWindow->Render();
    renderWindowInteractor->Start();
}
