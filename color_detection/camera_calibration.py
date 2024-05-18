import cv2
import numpy as np
import imutils

# https://medium.com/@sardorabdirayimov/colors-detection-using-masks-contours-in-opencv-72d127f0797e

# For documentation: Height: 480, width: 640
# Area threshold avoids noise
THRESHOLD = 1000
# Bounding box constants
BOX_START = (250, 150)
BOX_END = (400, 400)

colors = {
    "blue": [np.array([90, 175, 20]), np.array([130, 255, 255])],
    "red": [np.array([160, 165, 20]), np.array([180, 255, 255])],
    "green": [np.array([35, 40, 20]), np.array([70, 255, 255])],
}


# To be used for calibrating color ranges
def show_mask(frame, points):
    return cv2.inRange(frame, points[0], points[1])  # create mask with boundaries


# To be used for calibrating area theshold
def show_contours(frame, points):
    mask = show_mask(frame, points)

    cnts, heirarchy = cv2.findContours(mask, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

    cnts = [cnt for cnt in cnts if cv2.contourArea(cnt) > THRESHOLD]

    mask = cv2.cvtColor(mask, cv2.COLOR_GRAY2BGR)

    return cv2.drawContours(mask, cnts, -1, (255, 0, 0), -1)


def main():
    cap = cv2.VideoCapture(0)

    ret, image = cap.read()
    height, width, channels = image.shape

    print(f"Height: {height}, width: {width}")

    while cap.isOpened():
        ret, image = cap.read()

        # cv2.imshow(
        #     "Masked img",
        #     show_mask(cv2.cvtColor(image, cv2.COLOR_BGR2HSV), colors["green"]),
        # )

        # Show bounding box
        image = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
        image = show_contours(image, colors["blue"])
        image = cv2.rectangle(image, BOX_START, BOX_END, (0, 255, 255), 10)

        cv2.imshow(
            "Contour filtered img",
            image,
        )

        cv2.waitKey(25)


if __name__ == "__main__":
    main()
