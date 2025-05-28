import cv2
import numpy as np
import os
import glob

def stitch_images(image_paths, output_path="panorama.jpg", downscale_factor=1.0):
    """
    Stitches a list of images into a panorama.

    Args:
        image_paths (list): List of paths to the images to be stitched.
                            Images should be roughly in order (e.g., left-to-right).
        output_path (str): Path to save the resulting panorama.
        downscale_factor (float): Factor to downscale images before stitching.
                                  1.0 means original size. 0.5 means half size.
                                  Useful for large images to save memory/time.
    Returns:
        bool: True if stitching was successful, False otherwise.
    """
    print("Loading images...")
    imgs = []
    for image_path in image_paths:
        img = cv2.imread(image_path)
        if img is None:
            print(f"Error: Could not load image {image_path}")
            return False

        if downscale_factor != 1.0:
            print(f"Downscaling {image_path} by factor {downscale_factor}")
            new_width = int(img.shape[1] * downscale_factor)
            new_height = int(img.shape[0] * downscale_factor)
            img = cv2.resize(img, (new_width, new_height), interpolation=cv2.INTER_AREA)
        imgs.append(img)

    if len(imgs) < 2:
        print("Error: At least two images are required for stitching.")
        return False

    print("Attempting to stitch images...")
    
    # 使用cv2.Stitcher_create()，在OpenCV 4.x中是这样
    # 如果你用的是旧版OpenCV 3.x, 可能是 cv2.createStitcher()
    try:
        stitcher = cv2.Stitcher_create()
    except AttributeError:
        # Fallback for older OpenCV versions
        try:
            stitcher = cv2.createStitcher()
        except AttributeError:
            print("Error: OpenCV Stitcher not found. Make sure you have opencv-contrib-python installed.")
            return False

    status, pano = stitcher.stitch(imgs)

    if status == cv2.Stitcher_OK:
        print("Stitching successful!")

        # --- 自动裁剪黑色边框 ---
        # 将图像转为灰度
        gray_pano = cv2.cvtColor(pano, cv2.COLOR_BGR2GRAY)
        
        # 创建一个二值图像，其中非黑色像素为白色，黑色像素为黑色
        # 注意：由于JPEG压缩等原因，黑色区域可能不是纯黑(0)，所以阈值设为1或稍大一些
        _, thresh_pano = cv2.threshold(gray_pano, 1, 255, cv2.THRESH_BINARY)
        
        # 查找轮廓
        contours, _ = cv2.findContours(thresh_pano, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        
        if contours:
            # 找到最大的轮廓，它应该是拼接后的图像区域
            largest_contour = max(contours, key=cv2.contourArea)
            x, y, w, h = cv2.boundingRect(largest_contour)
            
            # 裁剪图像
            pano_cropped = pano[y:y+h, x:x+w]
            print(f"Cropped panorama to dimensions: {pano_cropped.shape[1]}x{pano_cropped.shape[0]}")
            cv2.imwrite(output_path, pano_cropped)
            print(f"Panorama saved to {output_path}")

            # 可选：显示结果
            # cv2.imshow("Original Panorama", pano)
            # cv2.imshow("Cropped Panorama", pano_cropped)
            # cv2.waitKey(0)
            # cv2.destroyAllWindows()
            return True
        else:
            print("Warning: Could not find contours to crop. Saving original panorama.")
            cv2.imwrite(output_path, pano)
            print(f"Panorama saved to {output_path}")
            return True # Technically stitching worked, cropping failed

    elif status == cv2.Stitcher_ERR_NEED_MORE_IMGS:
        print("Stitching failed: Not enough keypoints detected. Try with different images or ensure good overlap.")
    elif status == cv2.Stitcher_ERR_HOMOGRAPHY_EST_FAIL:
        print("Stitching failed: Homography estimation failed. Ensure sufficient overlap and distinct features.")
    else:
        print(f"Stitching failed with error code: {status}")

    return False

if __name__ == "__main__":
    # --- 配置 ---
    image_folder = r"D:\dateXHY\STM32CubeMX\test2_folder"  # 替换为你的图片文件夹路径
    output_image_path = "stitched_panorama.jpg"
    
    # 支持的图片格式
    supported_formats = ('*.jpg', '*.jpeg', '*.png', '*.bmp', '*.tiff')
    
    # 获取文件夹中所有支持格式的图片文件路径
    image_files = []
    for fmt in supported_formats:
        image_files.extend(glob.glob(os.path.join(image_folder, fmt)))
    
    # 确保图片按名称排序（假设名称反映了顺序，例如 img1.jpg, img2.jpg）
    image_files.sort() 

    if not image_files:
        print(f"No images found in folder: {image_folder}")
    elif len(image_files) < 2:
        print(f"Found only {len(image_files)} image(s). Need at least 2 to stitch.")
    else:
        print(f"Found images: {image_files}")
        
        # 调整下采样因子，如果图片很大，可以设为 0.5 或更小以加快处理速度和减少内存占用
        # 如果拼接效果不好，可以尝试使用原始尺寸 (downscale_factor=1.0)
        success = stitch_images(image_files, output_image_path, downscale_factor=1) 
        
        if success:
            print("Panorama generation process completed.")
        else:
            print("Panorama generation failed.")