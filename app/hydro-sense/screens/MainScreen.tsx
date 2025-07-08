// screens/MainScreen.tsx
import React from 'react';
import { View, Text, ScrollView, Button } from 'react-native';
import { useWaterLevel } from '../hooks/useWaterLevel';
import HistoryChart from '../components/HistoryChart';
import { useIsFocused } from '@react-navigation/native';

export default function MainScreen({ navigation, route }: any) {
  // receive settings via route.params or default
  const { length=100, width=100, shape='cubical' } = route.params || {};
  const { level, volume, rate, leak } = useWaterLevel({ length, width }, shape);
  const focused = useIsFocused(); // re-render on focus

  return (
    <ScrollView contentContainerStyle={{ padding: 20 }}>
      <Text style={{ fontSize: 24, textAlign: 'center' }}>Hydrosense</Text>
      <Text style={{ fontSize: 48, textAlign: 'center', marginVertical: 20 }}>
        {level.toFixed(1)} cm
      </Text>
      <Text style={{ fontSize: 20, textAlign: 'center' }}>
        Vol: {volume.toFixed(0)} L
      </Text>
      <Text style={{ marginTop: 10 }}>Usage Rate: {rate.toFixed(1)} L/h</Text>
      {leak && (
        <Text style={{ color: 'red', marginTop: 10 }}>⚠️ Possible leak detected!</Text>
      )}
      <Button title="Settings" onPress={() => navigation.navigate('Settings', route.params)} />
      <View style={{ height: 300, marginTop: 20 }}>
        <HistoryChart deviceParams={route.params} focused={focused} />
      </View>
    </ScrollView>
  );
}
