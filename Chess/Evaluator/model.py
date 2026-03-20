import os
import random
from uuid import uuid4
import tensorflow as tf
import numpy as np
import keras
from collections import deque
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3' 


class Evaluator:
    def __init__(self, model_path="models/chekpoint.keras", state_size=67, gamma=0.95, learning_rate=0.001, buffer_size=10000):
        self.id = uuid4()
        self.model_path=model_path
        self.state_size = state_size
        self.gamma = gamma
        self.learning_rate = learning_rate
        self.replay_buffer = deque(maxlen=buffer_size)
        os.makedirs("models", exist_ok=True)
        self.save_path = f"models/{self.id}.keras"

        self.model: keras.Sequential = self._build_model()
        self.load()
        self.model.summary()

    def _build_model(self) -> keras.Sequential:
        model = keras.Sequential([
            keras.layers.Input(shape=(self.state_size,), dtype=("int32")),
        ])
        for _ in range(4):
            model.add(keras.layers.Dense(128))
            model.add(keras.layers.LeakyReLU(alpha=0.01))
            model.add(keras.layers.BatchNormalization())
            model.add(keras.layers.Dropout(0.2))
        model.add(keras.layers.Dense(1))

        model.compile(optimizer="adam", loss="mse", metrics=["mse"])
        return model

    def remember(self, state, reward, next_state, done):
        self.replay_buffer.append((state, reward, next_state, done))

    def replay(self, batch_size=32):
        if len(self.replay_buffer) < batch_size:
            return

        minibatch = random.sample(self.replay_buffer, batch_size)

        for state, reward, next_state, done in minibatch:
            state = np.array([ord(char) for char in state])
            state.resize(1, self.state_size)
            target = self.model.predict(state)

            if done:
                target[0][0] = reward
            else:
                next_state_input = np.array(next_state).reshape(1, self.state_size)
                future_value = self.model.predict(next_state_input)[0][0]
                target[0][0] = reward + self.gamma * future_value

            self.model.fit(state, target, epochs=1, verbose=0)


    def save(self):
        path = self.model_path
        self.model.save(path)

    def load(self, input_path=None):
        path = input_path if input_path else self.model_path
        if os.path.exists(path):
            self.model = keras.models.load_model(path)
        else:
            print("No model to load")

    def evaluate(self, pos: str):
        input = np.array([ord(char) for char in pos])
        input.resize(1, self.state_size)
        return float(self.model.predict(input, verbose=0)[0][0])

eva = Evaluator()
