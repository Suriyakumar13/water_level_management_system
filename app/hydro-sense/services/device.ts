// services/device.ts
import AsyncStorage from '@react-native-async-storage/async-storage';
import { v4 as uuidv4 } from 'uuid';

const DEVICE_KEY = 'device_id';

export async function getOrCreateDeviceId(): Promise<string> {
  let id = await AsyncStorage.getItem(DEVICE_KEY);
  if (!id) {
    id = uuidv4();
    await AsyncStorage.setItem(DEVICE_KEY, id);
  }
  return id;
}
