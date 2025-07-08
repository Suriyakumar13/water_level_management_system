// screens/SettingsScreen.tsx
import React, { useState, useEffect } from 'react';
import { View, Text, TextInput, Button } from 'react-native';
import AsyncStorage from '@react-native-async-storage/async-storage';

const STORAGE_KEY = 'tank_config';

export default function SettingsScreen({ navigation, route }: any) {
  const [length, setLength] = useState('100');
  const [width, setWidth]   = useState('100');
  const [shape, setShape]   = useState<'cubical'|'cylindrical'>('cubical');

  useEffect(() => {
    (async () => {
      const json = await AsyncStorage.getItem(STORAGE_KEY);
      if (json) {
        const cfg = JSON.parse(json);
        setLength(cfg.length.toString());
        setWidth((cfg.width || cfg.length).toString());
        setShape(cfg.shape);
      }
    })();
  }, []);

  const save = async () => {
    const cfg = { length: +length, width: +width, shape };
    await AsyncStorage.setItem(STORAGE_KEY, JSON.stringify(cfg));
    navigation.navigate('Main', cfg);
  };

  return (
    <View style={{ padding: 20 }}>
      <Text>Length (cm)</Text>
      <TextInput value={length} onChangeText={setLength} keyboardType="numeric" />
      {shape === 'cubical' && (
        <>
          <Text>Width (cm)</Text>
          <TextInput value={width} onChangeText={setWidth} keyboardType="numeric" />
        </>
      )}
      <Text>Shape</Text>
      <View style={{ flexDirection: 'row', marginVertical: 10 }}>
        {(['cubical','cylindrical'] as const).map(s => (
          <Button
            key={s}
            title={s}
            onPress={() => setShape(s)}
            color={shape===s ? undefined : 'gray'}
          />
        ))}
      </View>
      <Button title="Save" onPress={save} />
    </View>
  );
}
