import cv2
import numpy as np
import paho.mqtt.client as mqtt
from tensorflow.keras.applications.mobilenet_v2 import preprocess_input
from tensorflow.keras.models import load_model

# Load the pre-trained MobileNetV2 model for mask detection
model = load_model("model.h5")

# MQTT broker details
mqtt_broker = "broker.emqx.io"
mqtt_port = 1883
mqtt_topic = "mask_alert"

# MQTT client setup
mqtt_client = mqtt.Client()

def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    client.subscribe(mqtt_topic)


def on_message(client, userdata, msg):
    print(msg.topic + " " + str(msg.payload))


mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message

# Connect to MQTT broker
mqtt_client.connect(mqtt_broker, mqtt_port, 60)

# Laptop camera setup
video_capture = cv2.VideoCapture(0)

# Load the face detector
face_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_frontalface_default.xml')

while True:
    # Capture video from laptop camera
    ret, frame = video_capture.read()

    # Convert the frame to grayscale for face detection
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

    # Detect faces in the frame
    faces = face_cascade.detectMultiScale(gray, scaleFactor=1.1, minNeighbors=5, minSize=(30, 30))

    # Iterate through detected faces
    for (x, y, w, h) in faces:
        face_roi = frame[y:y + h, x:x + w]
        face_input = cv2.resize(face_roi, (224, 224))
        face_input = np.expand_dims(face_input, axis=0)
        face_input = preprocess_input(face_input)

        # Perform mask detection using the pre-trained model
        predictions = model.predict(face_input)
        mask_probability = predictions[0][0]
        no_mask_probability = predictions[0][1]

        # Draw rectangles around the detected faces
        cv2.rectangle(frame, (x, y), (x + w, y + h), (255, 0, 0), 2)

        # If the probability of not wearing a mask is higher than wearing a mask, send an alert to MQTT server
        if no_mask_probability > mask_probability:
            mqtt_client.publish(mqtt_topic, "Mask not detected!")

    # Display the frame with detected faces
    cv2.imshow('Mask Detection', frame)

    # Exit the loop when 'q' is pressed
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

        m

# Release the video capture and close the window
video_capture.release()
cv2.destroyAllWindows()